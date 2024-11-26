#include "Stream.h"
#include "Server.h"

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
                            else if(host != "" && master.getHost() != "localhost" && master.getHost() != "127.0.0.1" && master.getHost() != host)
                            {
                                master.setMethod("INVALID");
                                break;
                            }
                            packetCreated = true;

                            if (master.getFileLen() && master.getFileLen() <= maxBodySize) {
                                path = "upload/" + master.getFileName();
                                if (master.isMethod() == POST) {
                                    struct stat mStat;
                                    if (!stat(path.c_str(), &mStat) && mStat.st_size > 0) {
                                        remove(path.c_str());
                                    }
                                    out.open(path.c_str(), ios::out | ios::binary);
                                }
                                if (!out.is_open())
                                    continue;
                                offset = (size_t)master.getHeaderLen();
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
    // if(_statusCode == " 413 Request Entity Too Large")
    //     loadError(client, getPageDefault("413"), "413 Payload Too Large");
    // else
        contentMaker(client, "HTTP/1.1" + _statusCode, "keep-alive", stream.getStream(), stream.streamSize());
}

/* void Server::response(int client, string path, string protocol) {
	int method = master.isMethod();
    struct stat info;
    size_t  pos = path.rfind(".");
	Stream  stream(this, path);
    Location location;
    string fullPath;
    _contentMaker.setStatus(" 200 OK");
    static string LocationRoot;
    defineLocationPath(location, path, LocationRoot);
    defineFullPath(fullPath, location, extractURL(path));
    if(HandleErrors(client, protocol) == false)
    {
        if (transfer) {
            if (method != DELETE && method != INVALID_REQUEST) {
                mimeMaker(path);
                if (pos == string::npos) {
                    if (maxBodySize < master.getFileLen() && master.getFileLen() > 0) {
                        cout << "Entrou 413\n";
                        loadError(client, getPageDefault("413"), "413 Payload Too Large");
                        return;
                    }
                    if(stat(fullPath.c_str(), &info) == 0)
                    {
                        if(location.data.find("index") != location.data.end())
                            loadIndexPage(stream, location);
                        else
                            loadDirectoryPage(client, stream, fullPath);
                    }
                    else{
                        loadError(client, getPageDefault("404"), "404 Not Found");
                        return;
                    }
                }
                else {
                    if (method == POST && maxBodySize > master.getFileLen()) {
                        contentMaker(client, protocol + " 200 OK", "keep-alive", stream.getStream(), stream.streamSize());
                        path = "/413.html";
                        _contentMaker.setStatus(" 413 Content Too Large"); // ?
                    }
                    if ((pos = path.find(".")) != string::npos) {
                        if (path.find("?") != string::npos) {
                            if ((path.find(".php") != string::npos && path[pos + 4] != '?') ||
                                (path.find(".py") != string::npos && path[pos + 3])) {
                                loadError(client, getPageDefault("404"), "404 Not Found");
                                return;
                            }
                            else {
                                if (path.find(".php") != string::npos)
                                    path = path.substr(0, pos + 4);
                                else if (path.find(".py") != string::npos)
                                    path = path.substr(0, pos + 3);
                            }
                        } else {
                            if ((path.find(".php") != string::npos && path.length() > pos + 4) ||
                                (path.find(".py") != string::npos && path.length() > pos + 3)) {
                                loadError(client, getPageDefault("404"), "404 Not Found");
                                return;
                            }
                        }
                    }
                    if(LocationRoot != "")
                        stream.loadFile(LocationRoot + path);
                    else
                        stream.loadFile(root + path);
                }
            }
            else if (method == DELETE) {
                struct stat mStat;
                string file = root + path;
                if (!access(file.c_str(), F_OK) && !access(file.c_str(), R_OK | W_OK | X_OK)) {
                    if (!stat(file.c_str(), &mStat) && mStat.st_size > 0){
                        if (remove(file.c_str()) == -1) {
                            loadError(client, getPageDefault("500"), "500 Internal Server Error");
                            return;
                        }
                    }
                }
                else {
                    loadError(client, getPageDefault("403"), "403 Forbidden");
                    return;
                }
            }
            else
            {
                loadError(client, getPageDefault("405"), "405 Method Not Allowed");
                return;
            }
        }
        else {
            loadError(client, getPageDefault("408"), "408 Request Timeout");
            return;
        }
        contentMaker(client, protocol + _contentMaker.getStatus(), "keep-alive", stream.getStream(), stream.streamSize());
    }
} */

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
