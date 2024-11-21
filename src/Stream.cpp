#include "Stream.h"
#define BUFFER_SIZE 65536

Stream::Stream(void) : buffer(NULL), size(0) {}

Stream::Stream(string file) : buffer(NULL), size(0) {
    if (!file.empty())
        loadFile(file);
    else
        saveFile(file);
}

Stream::Stream(Server *server, string path) : buffer(NULL), size(0) {
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
    }

    if (!error.empty()) {
        loadFile(ServerRef->getPageDefault(error.substr(2, 4)));
        ServerRef->setStatusCode(error);
        return true;
    }
    return false;
}

void Stream::handleFile(string& file){
    // memset(buffer, 0, sizeof(buffer));
    cout << "File received: " << file << endl;
    ifstream in(file.c_str(), ios::binary | ios::ate);
    if (!in.is_open() || in.bad() || in.fail())
    {
        cout << "deu erro\n";
        throw(string(" 500 Internal Server Error"));
    }

    size = in.tellg();
    in.seekg(0, ios::beg);
    buffer = new char[size];
    if (!buffer)
    {
        cout << "deu erro\n";
        throw(string(" 500 Internal Server Error"));
    }

    in.read(reinterpret_cast<char *>(buffer), size);
    in.close();
    cout << "saiu\n";
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
        throw(string("500 Internal Server Error: Pipe creation failed"));
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw(string("500 Internal Server Error: Fork failed"));
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
        string content_length = ServerRef->getMethod() == POST ? "CONTENT_LENGTH=" + body.size() : "";

        char* envp[] = {
            (char*)request_method.c_str(),
            (char*)(ServerRef->getMethod() == GET ? query_string.c_str() : content_length.c_str()),
            NULL
        };

        execve(args[0], args, envp);

        free(args[0]);
        free(args[1]);
        delete[] args;

        perror("Script execution failed");
        exit(EXIT_FAILURE);
    }

    else {
        close(fd[1]);
        char data[128];
        string result;
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
                    throw(string("504 Gateway Timeout"));
                }
            } else if (wait_result == -1) {
                perror("waitpid failed");
                throw(string("500 Internal Server Error: Waitpid failed"));
            } else {
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    cerr << "Script execution failed!" << endl;
                    throw(string("500 Internal Server Error: Script execution failed"));
                }
                break;
            }

            count = read(fd[0], data, sizeof(data));
            if (count > 0) {
                result.append(data, count);
            } else if (count == -1) {
                perror("Error reading from pipe");
                throw(string("500 Internal Server Error: Error reading from pipe"));
            } else if (count == 0) {
                break;
            }
        }

        close(fd[0]);

        size = result.size();
        buffer = new char[size];
        if (!buffer) {
            cerr << "Buffer allocation failed!" << endl;
            throw(string("500 Internal Server Error: Buffer allocation failed"));
        }

        memcpy(buffer, result.c_str(), size);
    }
}

void Stream::loadFile(string file) {
    try{
        if(!handleErrors(file)){
            if (file.find(".php") == string::npos && file.find(".py") == string::npos) {
                handleFile(file);
            } else {
                handleCGI(file);
            }
        }
    } catch (string& e) {
        loadFile(ServerRef->getPageDefault(e));
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
