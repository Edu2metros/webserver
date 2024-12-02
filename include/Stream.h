#ifndef STREAM_H
#define STREAM_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstdlib>
#include <sys/wait.h>
#include "Server.h"


using namespace std;

class Stream{
    void	*buffer;
    size_t	size;
    Server* ServerRef;
    string  path;
    int     client; 
    string _bufferString;

public:
    Stream(void);
    Stream(string file);
    Stream(Server *server, string path);
    void    createStream(void *, size_t);
    void    setStream(void *, size_t);
    void	*getStream(void);
    int		streamSize(void);
    void    loadFile(string file);
    void	saveFile(string file);
    Stream & operator = (Stream &);
    ~Stream(void);
    char** generateEnv();
    bool handleErrors(string file);
    void handleFile(string& file);
    void handleCGI(string& file);
    string getQueryString();
    string getBufferString() { return _bufferString; }    
};

#endif
