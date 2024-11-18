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

ContentMaker& Stream::getContentMaker() {
    return ServerRef->getContentMaker();
}

bool Stream::handleErrors(string file){
    string error;

    if (access(file.c_str(), F_OK) == -1) 
        error = " 404 Not Found";
    else if (access(file.c_str(), R_OK) == -1)
        error = " 403 Forbidden";
    else if(access(file.c_str(), X_OK) == -1)
        error = " 403 Forbidden";

    if (!error.empty()) {
        ServerRef->loadErrorPage(*this, error.substr(1, 3));
        ServerRef->getContentMaker().setStatus(error);
        return true;
    }
    return false;
}

void Stream::handleFile(string& file){
    ifstream in(file.c_str(), ios::binary | ios::ate);
    if (!in.is_open() || in.bad() || in.fail())
        throw(string(" 500 Internal Server Error"));

    size = in.tellg();
    in.seekg(0, ios::beg);
    buffer = new char[size];
    if (!buffer)
        throw(string(" 500 Internal Server Error"));

    in.read(reinterpret_cast<char *>(buffer), size);
    in.close();
}

void Stream::handleCGI(string& file){
    map<string, string> formData = ServerRef->getFormData();
    for(map<string, string>::iterator it = formData.begin(); it != formData.end(); ++it)
        cout << it->first << " = " << it->second << endl;
    
    int fd[2];
    if (pipe(fd) == -1)
        throw(string(" 500 Internal Server Error"));

    pid_t pid = fork();
    if (pid == -1)
        throw(string(" 500 Internal Server Error"));
    
    else if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[1]);

        string body = "name=juliocesar&age=3000000";
        int pipe_stdin[2];
        if (pipe(pipe_stdin) == -1) {
            perror("pipe() failed");
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

        if(file.find(".php") != string::npos) {
            const char* argv[] = {"php", file.c_str(), NULL};
            string content_length = "CONTENT_LENGTH=" + body.size();
            char* envp[] = {
                (char*)content_length.c_str(),
                (char*)"REQUEST_METHOD=POST",
                NULL
            };

            execve("/usr/bin/php", const_cast<char* const*>(argv), const_cast<char* const*>(envp));
        } else if(file.find(".py") != string::npos) {
        const char* argv[] = {"python3", file.c_str(), NULL};
        string content_length = "CONTENT_LENGTH=" + body.size();
        char* envp[] = {
            (char*)content_length.c_str(),
            (char*)"REQUEST_METHOD=POST",
            NULL
        };

        execve("/usr/bin/python3", const_cast<char* const*>(argv), const_cast<char* const*>(envp));
        perror("Execução do script falhou!");
        exit(EXIT_FAILURE); }
    }

    else {
        close(fd[1]);
        char data[128];
        string result;
        ssize_t count;

        int timeout_seconds = 5;
        time_t start_time = time(NULL);
        bool timeout_reached = false;

        while (true) {
            cout << "esperando..." << endl;
            int status;
            pid_t wait_result = waitpid(pid, &status, WNOHANG);

            if (wait_result == 0) {
                if (difftime(time(NULL), start_time) >= timeout_seconds) {
                    kill(pid, SIGKILL);
                    cerr << "O script demorou demais e foi terminado." << endl;
                    timeout_reached = true;
                    break;
                }
            } else if (wait_result == -1) {
                perror("waitpid falhou");
                break;
            } else {
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    cerr << "Execução do script falhou!" << endl;
                }
                break;
            }

            count = read(fd[0], data, sizeof(data));
            if (count > 0) {
                result.append(data, count);
            } else if (count == -1) {
                perror("Erro ao ler do pipe");
                break;
            } else if (count == 0)
                break;
        }

        close(fd[0]);

        if (timeout_reached)
            throw(string(" 504 Gateway Timeout"));

        size = result.size();
        buffer = new char[size];
        if (!buffer){
            cerr << "Falha na alocação do buffer!" << endl;
            throw(string(" 500 Internal Server Error"));
        }

        memcpy(buffer, result.c_str(), size);
    }
}

void Stream::loadFile(string file) {
    file = "www/form.py";
    try{
        if(!handleErrors(file)){
            if (file.find(".php") == string::npos && file.find(".py") == string::npos) {
                handleFile(file);
            } else {
                handleCGI(file);
            }
        }
    } catch (string& e) {
        ServerRef->loadErrorPage(*this, e.substr(1, 3));
        ServerRef->getContentMaker().setStatus(e);
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
