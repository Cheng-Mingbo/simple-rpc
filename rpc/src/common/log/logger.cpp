//
// Created by cheng on 23-5-25.
//

#include <mutex>
#include <thread>
#include "logger.h"
#include "console_appender.h"
#include "file_appender.h"
#include "config.h"
#include "asynclogger.h"
#include "eventloop.h"

namespace rpc {

static std::shared_ptr<Logger> g_logger = nullptr;

Logger::Logger(LogLevel level, int type) : level_(level), type_(type) {
    if (type_ == 1) {
        appenders_.push_back(new ConsoleAppender());
    } else if (type_ == 2) {
        appenders_.push_back(new FileAppender());
        Config& config = Config::GetGlobalConfig();
        async_logger_ = std::make_shared<AsyncLogger>(this, config.log_file_name_ + "_rpc", config.log_path_, config.log_file_size_);
        async_app_logger_ = std::make_shared<AsyncLogger>(this, config.log_file_name_ + "_app", config.log_path_, config.log_file_size_);
    } else if (type_ == 3) {
        appenders_.push_back(new ConsoleAppender());
        appenders_.push_back(new FileAppender());
        Config& config = Config::GetGlobalConfig();
        async_logger_ = std::make_shared<AsyncLogger>(this, config.log_file_name_ + "_rpc", config.log_path_, config.log_file_size_);
        async_app_logger_ = std::make_shared<AsyncLogger>(this, config.log_file_name_ + "_app", config.log_path_, config.log_file_size_);
    }
    
   
}

Logger::~Logger() {
    for (auto& appender : appenders_) {
        delete appender;
    }
}

void Logger::init() {
    // 加入定时器
   timer_channel_ = std::make_shared<TimerChannel>(Config::GetGlobalConfig().log_sync_interval_, true, [this] { syncLoop(); });
   auto* loop = EventLoop::getEventLoopOfCurrentThread();
   loop->addTimer(timer_channel_);
    LOG_DEBUG("success init logger");
}

void Logger::pushLog(const std::string &msg) {
    std::scoped_lock<std::mutex> lock(mutex_);
    for (auto& appender : appenders_) {
        appender->append(msg);
    }
}

void Logger::pushAppLog(const std::string &msg) {
    std::scoped_lock<std::mutex> lock(mutex_);
    for (auto& appender : appenders_) {
        appender->append_app(msg);
    }
}

void Logger::syncLoop() {
    LOG_DEBUG("syncLoop is called");
    std::vector<std::vector<std::string>> vecs(2);
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        for (auto& appender : appenders_) {
            appender->swap(vecs);
        }
    }
    if (!vecs[0].empty()) {
        async_logger_->append(vecs[0]);
    }
    if (!vecs[1].empty()) {
        async_app_logger_->append(vecs[1]);
    }
}

void Logger::InitGlobalLogger(int type) {
    g_logger = std::make_shared<Logger>(StringToLogLevel(Config::GetGlobalConfig().log_level_), type);
    if (type > 1) {
        g_logger->init();
    }
    
}

Logger &Logger::SetGlobalLogger(int type) {
    static std::once_flag flag;
    if (g_logger == nullptr) {
        std::call_once(flag, InitGlobalLogger, type);
    }
    return *g_logger;
}

}