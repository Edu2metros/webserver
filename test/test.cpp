#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>

struct Location {
    std::string path;
    std::map<std::string, std::string> directives;
};

struct ServerConfig {
    int listen;
    std::string index;
    std::string root;
    std::vector<Location> locations;
};

ServerConfig parseConfigFile(const std::string& filename) {
    ServerConfig config;
    std::ifstream file(filename.c_str());
    std::string line;

    Location currentLocation;
    bool inLocation = false;

    while (std::getline(file, line)) {
        // Remove leading/trailing whitespace
        std::string::size_type start = line.find_first_not_of(" \t\n\r\f\v");
        std::string::size_type end = line.find_last_not_of(" \t\n\r\f\v");
        if (start == std::string::npos || end == std::string::npos) {
            continue; // Skip empty lines
        }
        line = line.substr(start, end - start + 1);

        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }

        if (line.find("server {") != std::string::npos) {
            continue;
        } else if (line.find("}") != std::string::npos) {
            if (inLocation) {
                config.locations.push_back(currentLocation);
                currentLocation = Location();
                inLocation = false;
            }
        } else if (line.find("listen") != std::string::npos) {
            std::string value = line.substr(line.find(" ") + 1);
            try {
                config.listen = std::atoi(value.c_str());
            } catch (const std::exception& e) {
                std::cerr << "Invalid listen value: " << value << std::endl;
                throw;
            }
        } else if (line.find("index") != std::string::npos) {
            std::string value = line.substr(line.find(" ") + 1);
            if (inLocation) {
                currentLocation.directives["index"] = value;
            } else {
                config.index = value;
            }
        } else if (line.find("root") != std::string::npos) {
            std::string value = line.substr(line.find(" ") + 1);
            if (inLocation) {
                currentLocation.directives["root"] = value;
            } else {
                config.root = value;
            }
        } else if (line.find("autoindex") != std::string::npos) {
            std::string value = line.substr(line.find(" ") + 1);
            if (inLocation) {
                currentLocation.directives["autoindex"] = value;
            }
        } else if (line.find("location") != std::string::npos) {
            inLocation = true;
            currentLocation.path = line.substr(line.find(" ") + 1);
            if (!currentLocation.path.empty() && currentLocation.path[currentLocation.path.size() - 1] == '{') {
                currentLocation.path.erase(currentLocation.path.size() - 1); // Remove '{' at the end
            }
        } else {
            if (inLocation) {
                std::string::size_type spacePos = line.find(" ");
                std::string directive = line.substr(0, spacePos);
                std::string value = line.substr(spacePos + 1);
                currentLocation.directives[directive] = value;
            }
        }
    }
    file.close();
    return config;
}

int main() {
    try {
        ServerConfig config = parseConfigFile("autoindex.conf");

        std::cout << "Listen: " << config.listen << std::endl;
        std::cout << "Index: " << config.index << std::endl;
        std::cout << "Root: " << config.root << std::endl;

        for (std::vector<Location>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
            const Location& loc = *it;
            std::cout << "Location: " << loc.path << std::endl;
            for (std::map<std::string, std::string>::const_iterator dit = loc.directives.begin(); dit != loc.directives.end(); ++dit) {
                std::cout << "  " << dit->first << ": " << dit->second << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << std::endl;
    }

    return 0;
}
