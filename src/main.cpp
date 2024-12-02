#include "Server.h"
#include "Stream.h"

void handleSignal(int signal) {
    if (signal == SIGINT) {
        cout << "SIGINT signal received. Quitting..." << endl;
        exit(0);
    }
}

void webserver(const char *file) {
    vector<string> ports;
    Config config(file);
    size_t max = config.infoGet().size();
    vector<ServerInfo> info = config.infoGet();
    Server server[max];
    max = 0;
    set<string> usedPorts;
    for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it) {
        string port = it->port;

        if (usedPorts.find(port) == usedPorts.end()) {
            usedPorts.insert(port);
            server[max++] = Server(it->name, it->port, it->root, it->error, it->location, it->maxBodySize);
        } else
            cout << "Warning: Server for port " << port << " already exists. Ignoring duplicate configuration." << endl;
    }

    for (size_t i = 0; i < max; i++)
        Run(server, max);
}

int main(int argc, char **argv) {
    signal(SIGINT, handleSignal);  // Ctrl+C Signal
    if(argc != 1 && argc != 2)
    {
        cout << "Usage: ./webserv [config_file]" << endl;
        return (1);
    }

    try{
        if(argc == 2)
            webserver(argv[1]);
        else
            webserver("Configs/default.conf");
    }
    catch(const exception &e) {
        webserver("Configs/default.conf");
    }
    return (0);
}