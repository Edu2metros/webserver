#include "Server.h"
#include "Stream.h"
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

struct Location
{
    string path;
    map<string, string> directives;
};

class Server2
{
public:
    string port;
    string host;
    vector<Location> locations;
};

static const std::string WHITESPACE = " \t\n\r\f\v";
static const std::string COMMENT = "#";
static const std::string OPEN_BRACKET = "{";
static const std::string CLOSE_BRACKET = "}";

bool skipEmptyLines(std::string& line, int &bracketCount) {
    std::string::size_type start = line.find_first_not_of(WHITESPACE);
    std::string::size_type end = line.find_last_not_of(WHITESPACE);
    if (start == std::string::npos || end == std::string::npos) {
        return true;
    }
    line = line.substr(start, end - start + 1);
    if (line.empty() || line[0] == COMMENT[0]) {
        return true;
    }
    if (line.find("server {") != std::string::npos) {
        bracketCount++;
        return true;
    }
    if (line.find(OPEN_BRACKET) != std::string::npos) {
        bracketCount++;
    }
    return false;
}

void validateSemicolon(const std::string& line) {
    if (line.find(";") == std::string::npos && 
        line.find(OPEN_BRACKET) == std::string::npos &&
        line.find(CLOSE_BRACKET) == std::string::npos) {
        throw std::runtime_error("Missing semicolon");
    }
}

void processDirective(const std::string& line, Server2& config, Location& currentLocation, bool inLocation) {
    std::string::size_type spacePos = line.find(" ");
    std::string directive = line.substr(0, spacePos);
    std::string value = line.substr(spacePos + 1);

    if (directive == "listen") {
        config.port = value;
    } else if (directive == "index" || directive == "root" || directive == "autoindex") {
        if (inLocation) {
            currentLocation.directives[directive] = value;
        } else {
            config.host = value;
        }
    } else {
        if (inLocation) {
            currentLocation.directives[directive] = value;
        } else {
            std::cerr << "Error: Unknown directive outside location: " << directive << std::endl;
            throw std::runtime_error("Unknown directive outside location");
        }
    }
}

Server2 parser(const char *file) {
    std::ifstream in(file);
    std::string line;
    Location currentLocation;
    Server2 config;
    bool inLocation = false;
    int bracketCount = 0;

    if (!in.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    while (std::getline(in, line)) {
        if (skipEmptyLines(line, bracketCount)) {
            continue;
        }
        if (line.find(CLOSE_BRACKET) != std::string::npos) {
            bracketCount--;
            if (inLocation) {
                config.locations.push_back(currentLocation);
                currentLocation = Location();
                inLocation = false;
            }
            continue;
        }
        validateSemicolon(line);
        if (line.find("location") != std::string::npos) {
            inLocation = true;
            currentLocation.path = line.substr(line.find(" ") + 1);
            if (!currentLocation.path.empty() && currentLocation.path[currentLocation.path.size() - 1] == OPEN_BRACKET[0]) {
                currentLocation.path.erase(currentLocation.path.size() - 1);
            }
        } else {
            processDirective(line, config, currentLocation, inLocation);
        }
    }
    in.close();
    if (bracketCount != 0) {
        std::cerr << "Error: Unmatched opening bracket." << std::endl;
        throw std::runtime_error("Unmatched opening bracket");
    }
    return config;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <config-file>" << endl;
        return 1;
    }

    try {
        Server2 server = parser(argv[1]);

        cout << "Port: " << server.port << endl;
        cout << "Host: " << server.host << endl;

        for (vector<Location>::const_iterator it = server.locations.begin(); it != server.locations.end(); ++it) {
            const Location& loc = *it;
            cout << "Location: " << loc.path << endl;
            for (map<string, string>::const_iterator dit = loc.directives.begin(); dit != loc.directives.end(); ++dit) {
                cout << "  " << dit->first << ": " << dit->second << endl;
            }
        }
    } catch (const exception& e) {
        cerr << "Error parsing config file: " << e.what() << endl;
        return 1;
    }

    return 0;
}
