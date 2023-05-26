//
// Created by cheng on 23-5-25.
//

#include "loglevel.h"
#include <string>

std::string LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOW";
    }
}

LogLevel StringToLogLevel(const std::string& level) {
    if (level == "UNKNOW") {
        return LogLevel::UNKNOW;
    } else if (level == "DEBUG") {
        return LogLevel::DEBUG;
    } else if (level == "INFO") {
        return LogLevel::INFO;
    } else if (level == "ERROR") {
        return LogLevel::ERROR;
    }
}