#include "Server.h"
#include "Stream.h"

void handleSignal(int signal) {
    if (signal == SIGINT) {
        cout << "SIGINT signal received. Quitting..." << endl;
        exit(0);
    }
}

int main(int argc, char **argv) {
    signal(SIGINT, handleSignal);  // Ctrl+C Signal
    try{
        if (argc > 1) {
            Config config(argv[1]);
            size_t  max = config.infoGet().size();
            vector<ServerInfo> info = config.infoGet();
            Server server[max];
            max = 0;

            for (vector<ServerInfo>::iterator it = info.begin(); it != info.end(); ++it)
                server[max++] = Server(it->name, it->port, it->root, it->error, it->location, it->maxBodySize);

            for (size_t i = 0; i < max; i++)
                Run(server, max);
        }
        else {
            Server server;
            server.run();
        }
    }
    catch(const exception &e) {
        cerr << e.what() << endl;
        Server server;
        server.run();
    }
    return (0);
}