#include "Server.h"
#include "Stream.h"

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

void processDirective(const std::string& line, Server& config, Location& currentLocation, bool inLocation) {
    std::string::size_type spacePos = line.find(" ");
    std::string directive = line.substr(0, spacePos);
    std::string value = line.substr(spacePos + 1);

    if (directive == "listen") {
        config.setPort(value);
    } else if (directive == "index" || directive == "root" || directive == "autoindex") {
        if (inLocation) {
            currentLocation.directives[directive] = value;
        } else {
            config.setHost(value);
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

void erase(std::string &line) {
    if(line.find(";") != std::string::npos)
        line.erase(line.find(";"), 1);
    if(line.find(OPEN_BRACKET) != std::string::npos)
        line.erase(line.find(OPEN_BRACKET), 1);
    if(line.find(CLOSE_BRACKET) != std::string::npos)
        line.erase(line.find(CLOSE_BRACKET), 1);
}

Server parser(const char *file, int argc) {
    if(argc == 1)
        return Server();
    std::ifstream in(file);
    std::string line;
    Location currentLocation;
    Server config;
    bool inLocation = false;
    int bracketCount = 0;

    if (!in.is_open()) {
        cerr << "Error: Could not open file. Using default settings." << endl;
        return Server();
    }
    while (std::getline(in, line)) {
        if (skipEmptyLines(line, bracketCount))
            continue;
        if (line.find(CLOSE_BRACKET) != std::string::npos) {
            bracketCount--;
            if (inLocation) {
                config.addLocation(currentLocation);
                currentLocation = Location();
                inLocation = false;
            }
            continue;
        }
        validateSemicolon(line);
        erase(line);
        if (line.find("location") != std::string::npos) {
            inLocation = true;
            currentLocation.path = line.substr(line.find(" ") + 1);
            if (!currentLocation.path.empty() && currentLocation.path[currentLocation.path.size() - 1] == OPEN_BRACKET[0]) {
                currentLocation.path.erase(currentLocation.path.size() - 1);
            }
        } else
            processDirective(line, config, currentLocation, inLocation);
    }
    in.close();
    if (bracketCount != 0) {
        std::cerr << "Error: Unmatched opening bracket. Using default settings." << std::endl;
        return Server();
    }
    if(config.getPort().empty())
        config.setPort("8080");
    if(config.getHost().empty())
        config.setHost("localhost");
    return(config);
}

int main(int argc, char **argv) {
    Server server = parser(argv[1], argc);
    server.run();
    return 0;
}
