//
// Created by cheng on 23-5-25.
//

#ifndef RPC_LOGLEVEL_H
#define RPC_LOGLEVEL_H

#include <string>

enum class LogLevel {
    UNKNOW = 0,
    DEBUG = 1,
    INFO = 2,
    ERROR = 3,
};

std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string& level);

#endif //RPC_LOGLEVEL_H
