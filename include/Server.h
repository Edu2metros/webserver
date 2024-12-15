#ifndef SERVER_H
#define SERVER_H

#include <csignal>
#include "Config.h"
#include "Protocol.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <ctime>
#include <vector>
#include <map>
#include <algorithm>
#include <dirent.h>
#include <sys/types.h>
#include <set>

class Stream;

typedef struct sockaddr_in SockAddrIn;
using namespace std;

class Server
{
public:
    Server(void);
    Server(char *);
    Server(Server const &);
    Server(string host, string port, string root, map<string, string> error, vector<Location> location, size_t maxBodySize);
    int serverSocket(int);
    string createPacket(int);
    void requestTreat(int, string);
    string mimeMaker(string);
    void contentMaker(int, string, string, void *, size_t);
    void response(int, string, string);
    void postPrepare(string);
    void run(void);
    Server &operator=(Server const &);
    ~Server(void);

    vector<Location> getLocations() const;
    vector<Location>::iterator getBegin();
    vector<Location>::iterator getEnd();
    vector<Location>::const_iterator getBegin() const;
    vector<Location>::const_iterator getEnd() const;

    void setPort(string &);
    string getPort() const;

    void setRoot(const string &root);
    string getRoot() const;

    void setHost(const string &host);
    string getHost() const;

    void addLocation(const Location &location);
    Location findLocationPath(const string &name);
    string findDirectiveName(const string &name);
    string findDirectiveValue(const string &path);
    void setMaxBodySize(const string &size);
    void addErrorPage(const string &error, const string &path);
    void addErrorPage(const string &error, const string &path, struct Location &location);
    string getErrorPage(const string &error);
    string getErrorPage(const string &error, struct Location &location);
    void handleDeleteMethod(const string &path);
    string adjustScriptPath(const string &path);
    void loadIndexPage(Stream &stream, Location &location);
    // void loadErrorPage(Stream &stream, const string &errorCode);
    void loadDirectoryPage(int client, Stream &stream, const std::string &fullPath);
    void execute(int socket);
    void checkAcceptedMethod(Protocol &master);
    void defineFullPath(string &fullPath, Location &location, string url);
    void defineLocationPath(Location &location, string path, string &LocationRoot);
    void LoadSpecifiedFile(int client, const string &path, const string &status);
    bool HandleErrors(int client, string protocol, Stream& stream);
    string getPageDefault(const string &errorCode);
    void loadError(int client, std::string filePath, const std::string &errorCode);
    void printErrors(const std::vector<std::string> &codeErrors)
    {
        for (std::vector<std::string>::const_iterator it = codeErrors.begin(); it != codeErrors.end(); ++it)
        {
            std::map<std::string, std::string>::const_iterator errIt = error.find(*it);
            if (errIt != error.end())
            {
                std::cout << "Erro: " << errIt->second << std::endl;
            }
            else
            {
                std::cout << "Erro: Código \"" << *it << "\" não encontrado." << std::endl;
            }
        }
    }
    int getMethod() { return (master.isMethod()); }
    string getMime() const { return mime; }
    string getContentBody() const { return master.getContentBody(); }

    void handleGetPost(int client, string &path, Stream &stream);
    void handleDelete(int client, Stream &stream, const std::string &fullPath, Location &location);
    string getStatusCode() const { return _statusCode; }
    void setStatusCode(const string &statusCode) { _statusCode = statusCode; }
    void contentMaker(int client, string protocol, string connection, string buffer);
    void checkServerName(Protocol &master);
    void setError(const string& error, const string& msg, bool& readyToWrite);
    
protected:
    string host;
    string port;
    size_t maxBodySize;
    int sock;
    string root;
    string mime;
    bool transfer;
    map<string, string> error;
    vector<Location> location;
    map<string, string> errorPages;
    Protocol master;
    vector<string> ports;
    string _statusCode;
};

void parser(const char *file, Server &config);
void printLocations(const Server &config);
string extractURL(string &path);
std::string ft_strip(const std::string &s);
void Run(Server *server, int max);
void handleSignal(int signal);
void trim(string &str);
string returnTrim(const string& str);
void trim(string& str);
void trim(char str[]);
#endif
