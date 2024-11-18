#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class Logger {
private:
    std::ofstream logFile;

    std::string getTimestamp() {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
        return std::string(buf);
    }

public:
    Logger(const std::string& filename) {
        logFile.open(filename.c_str(), std::ios::app);
        if (!logFile.is_open())
            std::cerr << "Could not open log file: " << filename << std::endl;
    }

    ~Logger() {
        if (logFile.is_open())
            logFile.close();
    }

    void log(const std::string& message) {
        std::string timestamp = getTimestamp();
        std::string logMessage = "[" + timestamp + "] " + message;
        
        if (logFile.is_open())
            logFile << logMessage << std::endl;
        
        std::cout << logMessage << std::endl;
    }
};

#endif