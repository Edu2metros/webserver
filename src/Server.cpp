#include "Stream.h"
#include "Server.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>

using namespace std;

string Server::createPacket(int client) {
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
		}
		else {
            timeout.tv_sec = 0;
			timeout.tv_usec = master.getFileLen() / 1000;
		}

        if (master.isMethod() == INVALID_REQUEST || master.isMethod() == ENTITY_TOO_LARGE || master.isMethod() == INVALID_HOST) {
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
        }

        int receiving = select(client + 1, &read_fd, NULL, NULL, &timeout);
        if (receiving < 0) {
            creating = false;
            transfer = false;
            cout << "Error on select\n";
        }
        else if (receiving == 0) {
            transfer = true;
            break;
        }
        else {
            int countPiece = 0;
            if (FD_ISSET(client, &read_fd)) {
                while (true) {
                    memset(buffer, 0, sizeof(buffer));
                    piece = recv(client, buffer, 65535, 0);
                    countPiece += piece;
                    cout << "received " << piece << " bytes\n";
                    if (piece > 0) {
                        master.extract(buffer);
                        if(piece != master.getFileLen())
                            cout << "foi\n";
                        currentSize += piece;
                        if (!packetCreated && !out.is_open()) {
                            checkAcceptedMethod(master);
							if (master.isMethod() == INVALID_REQUEST)
								break;
                            else if(host != "" && master.getHost() != "localhost" && master.getHost() != "127.0.0.1" && master.getHost() != host)
                            {
                                master.setMethod("INVALID");
                                break;
                            }
                            if(piece == master.getFileLen())
                            {
                                cout << "AAAAAAAAAAAAAAAAAAAA entrou\n";
                                packetCreated = true;
                            }

                            if (master.getFileLen() && packetCreated == true) {
                                path = "upload/" + master.getFileName();
                                if(access(path.c_str(), F_OK) == 0)
                                    path = path + "_copy";
                                if (master.isMethod() == POST) {
                                    // struct stat mStat;
                                    // if (!stat(path.c_str(), &mStat) && mStat.st_size > 0) {
                                        // remove(path.c_str());
                                    // }
                                    out.open(path.c_str(), ios::out | ios::binary);
                                }
                                if (!out.is_open())
                                    continue;
                                offset = (size_t)master.getHeaderLen();
                                cout << "Header length: " << offset << endl;
                            }
                        }
                        if(master.getFileLen() != 0 && master.isMethod() == POST && master.getFileLen() > maxBodySize)
                        {
                            master.setMethod("ENTITY_TOO_LARGE");
                            break;
                        } 
                        dataLen = piece - offset;
                        size_t remainingLen = size_t(master.getFileLen()) - writtenByte;

                        if (remainingLen <= dataLen)
                            dataLen = remainingLen;
                        if (dataLen > 0) {
                            char *sub = strstr(buffer + offset, master.getBoundary().c_str());
                            cout << "Buffer + Offset: " << buffer + offset << endl;
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
                            break;
                        }
                        offset = 0;
                    }
                    else if (piece == 0 || writtenByte >= master.getFileLen()) {
                        cout << "Received entire file\n";
                        transfer = true;
                        creating = true;
                        break;
                    }
                    else {
                        transfer = false;
                        cout << "*uploaded " << writtenByte << " of " << master.getFileLen() << endl;
                        break;
                    }
                }
            }
        }
    }
    
    out.close();
    if (!transfer || master.isMethod() == ENTITY_TOO_LARGE) {
        remove(path.c_str());
        cout << "could not transfer " << path << endl;
    }
    return "";
}


void Server::loadError(int client, string filePath, const string& errorCode)
{
    trim(filePath);
    ifstream file(filePath.c_str());

    string errorPage;
    string statusCode;
    
    if (access(filePath.c_str(), F_OK) == -1) {
        errorPage = getPageDefault("404");
        statusCode = "404 Not Found";
    }
    else if (access(filePath.c_str(), R_OK) == -1) {
        errorPage = getPageDefault("403");
        statusCode = "403 Forbidden";
    }
    else if (!file) {
        errorPage = getPageDefault("500");
        statusCode = "500 Internal Server Error";
    }

    if (!statusCode.empty()) {
        loadError(client, errorPage, statusCode);
        return;
    }

    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    ostringstream responseStream;
    responseStream << "HTTP/1.1 " << errorCode << " Error\r\n"
                   << "Connection: keep-alive\r\n"
                   << "Content-Type: text/html\r\n"
                   << "Content-Length: " << content.size() << "\r\n\r\n"
                   << content;
    
    string response = responseStream.str();

    if (send(client, response.c_str(), response.size(), 0) == -1) {
        loadError(client, getPageDefault("500"), "500 Internal Server Error");
    }
}

void Server::handleDelete(int client, Stream &stream, const string &fullPath, Location &location) {
    struct stat mStat;
    string file = root + master.getPath();

    if (stat(file.c_str(), &mStat) != 0)
        _statusCode = " 404 Not Found";
    else if (access(file.c_str(), R_OK | W_OK) != 0)
        _statusCode = " 403 Forbidden";
    else if (remove(file.c_str()) != 0)
        _statusCode = " 500 Internal Server Error";

    stream.loadFile(getPageDefault(_statusCode.substr(2, 4)));
}

void Server::handleGetPost(int client, string& path, Stream &stream)
{   
    static string LocationRoot;
    struct stat info;
    Location location;
    string fullPath;
    size_t pos = path.rfind(".");

    mimeMaker(path);
    defineLocationPath(location, path, LocationRoot);
    defineFullPath(fullPath, location, extractURL(path));
    if(stat(fullPath.c_str(), &info) == 0)
    {
        if(pos != string::npos){
            if(LocationRoot != "")
                stream.loadFile(LocationRoot + path);
            else
                stream.loadFile(root + path);
        }
        else if(location.data.find("index") != location.data.end())
            loadIndexPage(stream, location);
        else if(S_ISDIR(info.st_mode))
            loadDirectoryPage(client, stream, fullPath);
    }
    else{
        _statusCode = " 404 Not Found";
        stream.loadFile(getPageDefault("404"));
    }
}

void Server::contentMaker(int client, string protocol, string connection, string buffer)
{
    if(_statusCode[0] != ' ')
        _statusCode = ' ' + _statusCode;
    ostringstream responseStream;
    responseStream << protocol + _statusCode << "\r\n"
                   << "Connection: " << connection << "\r\n"
                   << "Content-Type: text/html\r\n"
                   << "Content-Length: " << buffer.size() << "\r\n\r\n"
                   << buffer;

    string response = responseStream.str();
    send(client, response.c_str(), response.size(), 0);
}

void Server::response(int client, string path, string protocol){
    static string LocationRoot;
    struct stat info;
    size_t pos = path.rfind(".");
    Stream stream(this, path);
    Location location;
    string fullPath;
    _statusCode = " 200 OK";

    defineLocationPath(location, path, LocationRoot);
    defineFullPath(fullPath, location, extractURL(path));
    if(HandleErrors(client, protocol, stream) == false){
        if(master.isMethod() == GET || master.isMethod() == POST)
            handleGetPost(client, path, stream);
        else
            handleDelete(client, stream, fullPath, location);
    }
    contentMaker(client, protocol, "keep-alive", stream.getBufferString());
}

void Server::execute(int socket) {
    int client = accept(socket, NULL, NULL);
    if (client == -1)
        return ;
    fcntl(client, F_SETFL, O_NONBLOCK);
    createPacket(client);
	response(client, master.getPath(), master.getType());
    close(client);
}

void Server::run(void) {
	int		sock;

	if ((sock = serverSocket(SOCK_STREAM)) == -1)
		exit(-1);
	while (1)
        execute(sock);
}

void    Run(Server *server, int max) {
    int sock[max];
    for (int i = 0; i < max; i++) {
        if ((sock[i] = server[i].serverSocket(SOCK_STREAM)) == -1)
            exit(-1);
    }
    while (1) {
        for (int i = 0; i < max; i++)
            server[i].execute(sock[i]);
    }
}
