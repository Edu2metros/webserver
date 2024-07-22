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

typedef struct sockaddr_in SockAddrIn;
using namespace std;

class Server {
	public:
		Server(void);
    Server(char *);
		Server(Server const &);
		void run(void);
		void sender(int, string);
		std::string receiver(int, int);
		Server & operator = (Server const &);
		~Server(void);
	private:
		string	   host;
    	string     port;
		SockAddrIn address;
		int        sock;
};

#endif
