//
// Created by cheng on 23-5-25.
//
#include <semaphore>
#include <string>
#include <sstream>
#include "logevent.h"
#include "util.h"

namespace rpc {
std::string LogEvent::toString() {
    struct timeval now_time{};
    gettimeofday(&now_time, nullptr);
    struct tm now_time_t{};
    localtime_r(&now_time.tv_sec, &now_time_t);
    char time_str[128];
    strftime(time_str, 128, "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string now_time_str(time_str);
    long ms = now_time.tv_usec / 1000;
    now_time_str = now_time_str + "." + std::to_string(ms);
    pid_ = getPid();
    tid_ = getThreadId();
    
    std::stringstream ss;
    ss << "[" << LogLevelToString(level_) << "]\t" << "["
       << now_time_str << "]\t"
       << "[" <<  pid_ << ":" << tid_ << "]\t";
    return ss.str();
}
}