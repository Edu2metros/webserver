#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
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
#include <vector>
#include <string>
#include <map>

typedef struct sockaddr_in SockAddrIn;
using namespace std;

struct Location {
    string path;
    map<string, string> directives;
};

class Server {
    public:
        Server(void);
        Server(char *);
        Server(Server const &);
        void run(void);
        void sender(int, string);
        string receiver(int, int);
        Server & operator = (Server const &);
        ~Server(void);

        void setPort(const string& port) { this->port = port; }
        string getPort() const { return port; }

        void setHost(const string& host) { this->host = host; }
        string getHost() const { return host; }

        void addLocation(const Location& location) { locations.push_back(location); }
        
        vector<Location> getLocations() const { return locations; }
        vector<Location>::iterator getBegin() { return locations.begin(); }
        vector<Location>::iterator getEnd() { return locations.end(); }
        vector<Location>::const_iterator getBegin() const { return locations.begin(); }
        vector<Location>::const_iterator getEnd() const { return locations.end(); }

    private:
        string host;
        string port;
        SockAddrIn address;
        int sock;
        vector<Location> locations;
};

#endif
