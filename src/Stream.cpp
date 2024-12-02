#include "Stream.h"
#define BUFFER_SIZE 65536

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
    if (!file.empty())
        loadFile(file);
    else
        saveFile(file);
}

Stream::Stream(Server *server, string path) : buffer(NULL), _bufferString(""), size(0) {
    ServerRef = server;
    this->path = path;
}

void    Stream::createStream(void *data, size_t len) {
    size = len;
    buffer = new char[size];
    if (!buffer)
        return ;
    memcpy(buffer, data, size);
    ((char *)buffer)[size - 1] = '\0';
}

void	*Stream::getStream(void) {
    return buffer;
}

int		Stream::streamSize(void) {
    return size;
}

bool Stream::handleErrors(string file){
    trim(file);
    string error;

    if (access(file.c_str(), F_OK) == -1) 
        error = " 404 Not Found";
    else if (access(file.c_str(), R_OK) == -1)
        error = " 403 Forbidden";
    else if(access(file.c_str(), X_OK) == -1 && (file.find(".php") != string::npos || file.find(".py") != string::npos))
        error = " 403 Forbidden";

    if(!error.empty())
    {
        cout << file << endl;
        cout << "chamado aqui: " << error << endl;
        cout << "file: " << file << endl;
    }

    if (!error.empty()) {
        loadFile(ServerRef->getPageDefault(error.substr(1, 3)));
        ServerRef->setStatusCode(error);
        return true;
    }
    return false;
}


void Stream::handleFile(string& file){
    trim(file);
    ifstream infile(file.c_str());
    if(!infile.is_open())
    {
        cerr << "Could not open file\n";
        throw(string(" 500 Internal Server Error"));
    }

    _bufferString = string((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    if (_bufferString.empty())
    {
        cerr << "Could not read file\n";
        throw(string(" 500 Internal Server Error"));
    }
}

string Stream::getQueryString(){
    if(path.find("?") != string::npos){
        size_t pos = path.find("?");
        return path.substr(pos + 1);
    }
    return "";
}

void Stream::handleCGI(string& file) {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        throw(string(" 500 Internal Server Error"));
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        throw(string(" 500 Internal Server Error"));
    }

    else if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[1]);

        string body = ServerRef->getContentBody();
        int pipe_stdin[2];
        if (pipe(pipe_stdin) == -1) {
            perror("Pipe creation for STDIN failed");
            exit(EXIT_FAILURE);
        }

        if (fork() == 0) {
            close(pipe_stdin[0]);
            write(pipe_stdin[1], body.c_str(), body.size());
            close(pipe_stdin[1]);
            exit(0);
        } else {
            close(pipe_stdin[1]);
            dup2(pipe_stdin[0], STDIN_FILENO);
            close(pipe_stdin[0]);
        }

        char **args = new char*[3];
        if (file.find(".php") != string::npos) {
            args[0] = strdup("/usr/bin/php");
            args[1] = strdup(file.c_str());
            args[2] = NULL;
        } else if (file.find(".py") != string::npos) {
            args[0] = strdup("/usr/bin/python3");
            args[1] = strdup(file.c_str());
            args[2] = NULL;
        } else {
            cerr << "Unsupported script type" << endl;
            exit(EXIT_FAILURE);
        }

        string request_method = std::string("REQUEST_METHOD=") + (ServerRef->getMethod() == GET ? "GET" : "POST");  
        string query_string = ServerRef->getMethod() == GET ? "QUERY_STRING=" + getQueryString() : "";
        string content_length = ServerRef->getMethod() == POST ? "CONTENT_LENGTH=" + std::to_string(body.size()) : "";

        // Variáveis de ambiente necessárias para PHP e Python
        char* envp[] = {
            (char*)request_method.c_str(),
            (char*)(ServerRef->getMethod() == GET ? query_string.c_str() : content_length.c_str()),
            NULL
        };

        // Executa o script
        execve(args[0], args, envp);

        // Limpa a memória em caso de erro
        free(args[0]);
        free(args[1]);
        delete[] args;

        perror("Script execution failed");
        exit(EXIT_FAILURE);
    }

    else {
        close(fd[1]);
        char data[128];
        _bufferString.clear();
        ssize_t count;

        int timeout_seconds = 5;
        time_t start_time = time(NULL);

        while (true) {
            int status;
            pid_t wait_result = waitpid(pid, &status, WNOHANG);

            if (wait_result == 0) {
                if (difftime(time(NULL), start_time) >= timeout_seconds) {
                    kill(pid, SIGKILL);
                    cerr << "The script took too long and was terminated." << endl;
                    throw(string(" 504 Gateway Timeout"));
                }
            } else if (wait_result == -1) {
                perror("waitpid failed");
                throw(string(" 500 Internal Server Error"));
            } else {
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    cerr << "Script execution failed!" << endl;
                    throw(string(" 500 Internal Server Error"));
                }
                break;
            }

            count = read(fd[0], data, sizeof(data));
            if (count > 0) {
                _bufferString.append(data, count);
            } else if (count == -1) {
                perror("Error reading from pipe");
                throw(string(" 500 Internal Server Error"));
            } else if (count == 0) {
                break;
            }
        }

        close(fd[0]);
    }
}


void Stream::loadFile(string file) {
    _bufferString.clear();
    try{
        if(!handleErrors(file)){
            if (file.find(".php") == string::npos && file.find(".py") == string::npos) {
                handleFile(file);
            } else {
                handleCGI(file);
            }
        }
    } catch (string& e) {
        loadFile(ServerRef->getPageDefault(e.substr(1, 3)));
        ServerRef->setStatusCode(e);
    }
}

void	Stream::saveFile(string file) {
    if (file.empty())
        return;
    ofstream out(file.c_str(), ofstream::out | ofstream::binary);

    if (!out.is_open() || out.bad() || out.fail())
        return;
    out.write(reinterpret_cast<char*>(buffer), size);
    if (!out) {
        cerr << "could not write file\n";
        return ;
    }
    out.close();
}

Stream & Stream::operator = (Stream & pointer) {
    if (this != &pointer) {
        buffer = reinterpret_cast<char*>(pointer.buffer);
        size = pointer.size;
    }
    return *this;
}

Stream::~Stream(void) {
    if (buffer)
        delete[] reinterpret_cast<char *>(buffer);
}


/* string Server::createPacket(int client) {
    fd_set          read_fd;
    struct timeval  timeout;
    bool            packetCreated = false;
    bool            creating = true;
    char            buffer[65535];
    size_t          currentSize = 0;
    size_t          writtenByte = 0;
    size_t          dataLen;
    size_t          offset = 0;
    int             piece;
    string          path("");
    ofstream        out;

    master.reset();
    transfer = false;

    while (creating) {
        FD_ZERO(&read_fd);
        FD_SET(client, &read_fd);

        if (!packetCreated || maxBodySize < master.getFileLen()) {
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
        } else {
            timeout.tv_sec = 0;
            timeout.tv_usec = master.getFileLen() / 1000;
        }

        if (master.isMethod() == INVALID_REQUEST || 
            master.isMethod() == ENTITY_TOO_LARGE || 
            master.isMethod() == INVALID_HOST ||
            master.isMethod() == CONFLICT) {
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
        }

        int receiving = select(client + 1, &read_fd, NULL, NULL, &timeout);
        if (receiving < 0) {
            creating = false;
            transfer = false;
            cout << "Error on select\n";
        } else if (receiving == 0) {
            transfer = true;
            break;
        } else {
            if (FD_ISSET(client, &read_fd)) {
                while (true) {
                    memset(buffer, 0, sizeof(buffer));
                    piece = recv(client, buffer, 65535, 0);
                    if (piece > 0) {
                        currentSize += piece;
                        if (!packetCreated && !out.is_open()) {
                            master.extract(buffer);
                            checkAcceptedMethod(master);
                            if (master.isMethod() == INVALID_REQUEST)
                                break;
                            else if (host != "" && master.getHost() != "localhost" &&
                                     master.getHost() != "127.0.0.1" && master.getHost() != host) {
                                master.setMethod("INVALID");
                                break;
                            }
                            packetCreated = true;

                            if (master.getFileLen() && master.getFileLen() <= maxBodySize) {
                                if (access("upload", F_OK | W_OK) == -1) {
                                    master.setMethod("CONFLICT");
                                    cerr << "Could not access upload directory\n";
                                    break;
                                }
                                path = "upload/" + master.getFileName();

                                // Abrindo o arquivo para escrita
                                if (master.isMethod() == POST) {
                                    out.open(path.c_str(), ios::out | ios::binary);
                                    if (!out.is_open()) {
                                        cerr << "Failed to open file for writing: " << path << endl;
                                        master.setMethod("CONFLICT");
                                        break;
                                    }
                                }
                                offset = (size_t)master.getHeaderLen();
                            }
                        }
                        if (master.getFileLen() != 0 && 
                            master.isMethod() == POST && 
                            master.getFileLen() > maxBodySize) {
                            master.setMethod("ENTITY_TOO_LARGE");
                            break;
                        }
                        dataLen = piece - offset;
                        size_t remainingLen = size_t(master.getFileLen()) - writtenByte;

                        if (remainingLen <= dataLen)
                            dataLen = remainingLen;
                        if (dataLen > 0) {
                            char *sub = strstr(buffer + offset, master.getBoundary().c_str());
                            if (sub)
                                dataLen -= master.getBoundary().length() + 6;
                            if (master.getFileLen() <= maxBodySize)
                                out.write(buffer + offset, dataLen);
                            writtenByte += dataLen;
                            if (master.isMethod() == POST)
                                cout << "uploaded " << writtenByte << " of " << master.getFileLen() << endl;
                        }
                        if (writtenByte >= master.getFileLen()) {
                            cout << "transfer done\n";
                            transfer = true;
                            creating = false;
                            break;
                        }
                        offset = 0;
                    } else if (piece == 0 || writtenByte >= master.getFileLen()) {
                        if (writtenByte >= master.getFileLen()) {
                            cout << "Received entire file\n";
                        } else {
                            cout << "Connection closed prematurely\n";
                        }
                        transfer = true;
                        creating = false;
                        break;
                    } else {
                        transfer = false;
                        cout << "*uploaded " << writtenByte << " of " << master.getFileLen() << endl;
                        break;
                    }
                }
            }
        }
    }

    // Fechando o arquivo após uso
    if (out.is_open())
        out.close();

    if (!transfer || master.isMethod() == ENTITY_TOO_LARGE) {
        usleep(master.getFileLen() / 1000);
        remove(path.c_str());
        cout << "could not transfer " << path << endl;
    }
    return "";
}
 */