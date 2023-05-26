//
// Created by cheng on 23-5-25.
//

#ifndef RPC_LOGGER_H
#define RPC_LOGGER_H

#include <memory>
#include <vector>
#include <cstdio>
#include "loglevel.h"
#include "appender.h"


#define LOG_DEBUG(str, ...) \
    do { \
        if (rpc::Logger::GetGlobalLogger().getLevel() <= LogLevel::DEBUG) { \
            rpc::Logger::GetGlobalLogger().pushLog(rpc::LogEvent(LogLevel::DEBUG).toString() \
            + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
            + rpc::formatString(str, ##__VA_ARGS__) + '\n'); \
        } \
    } while (0)

#define LOG_INFO(str, ...) \
    do { \
        if (rpc::Logger::GetGlobalLogger().getLevel() <= LogLevel::INFO) { \
            rpc::Logger::GetGlobalLogger().pushLog(rpc::LogEvent(LogLevel::INFO).toString() \
            + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
            + rpc::formatString(str, ##__VA_ARGS__) + "\n"); \
        } \
    } while (0)

#define LOG_ERROR(str, ...) \
    do {                    \
        if (rpc::Logger::GetGlobalLogger().getLevel() <= LogLevel::ERROR) { \
            rpc::Logger::GetGlobalLogger().pushLog(rpc::LogEvent(LogLevel::ERROR).toString() \
            + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
            + rpc::formatString(str, ##__VA_ARGS__) + '\n'); \
        } \
    } while (0)


namespace rpc {
class AsyncLogger;

template<typename... Args>
std::string formatString(const char *str, Args &&... args) {
    int size = snprintf(nullptr, 0, str, args...);
    std::string result;
    
    if (size > 0) {
        result.resize(size);
        snprintf(&result[0], size + 1, str, args...);
    }
    return result;
}





class Logger {
  public:
    using s_ptr = std::shared_ptr<Logger>;
    
    explicit Logger(LogLevel level, int type = 1);
    
    ~Logger();
    
    void pushLog(const std::string &msg);
    
    void pushAppLog(const std::string &msg);
    
    void init();
    
    [[nodiscard]] LogLevel getLevel() const { return level_; }
    
    void syncLoop();
  
  public:
    static Logger &GetGlobalLogger();
    
    static void InitGlobalLogger(int type = 1);
  
  private:
    LogLevel level_;
    std::vector<Appender *> appenders_;
    std::shared_ptr<AsyncLogger> async_logger_;
    std::shared_ptr<AsyncLogger> async_app_logger_;
    
    std::mutex mutex_;
    
    std::string file_name_;
    std::string file_path_;
    int file_size_{0};
    
    int type_{0};
    
};


    
    
}
#endif //RPC_LOGGER_H
