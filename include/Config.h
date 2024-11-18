#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <map>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

typedef struct Location{
    string path;
    map<string, string> data;
}Location;

typedef struct ServerInfo{
    string              name;
    string              root;
    string              port;
    size_t              maxBodySize;
    map<string, string> error;
    vector<Location>    location;

    ServerInfo() : name(""), root(""), port(""), maxBodySize(0) {}
}ServerInfo;

class Config{
    vector<ServerInfo> info;
    public:
        Config(void);
        Config(char *file);
        vector<ServerInfo> infoGet(void);
        string getName(int);
        void print(void);
        ~Config(void);
};

std::string ft_strip(const std::string& s);

#endif
