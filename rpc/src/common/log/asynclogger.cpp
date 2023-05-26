//
// Created by cheng on 23-5-25.
//

#include "asynclogger.h"
#include "logger.h"

#include <utility>
#include <sstream>

namespace rpc {
std::binary_semaphore AsyncLogger::sem_(0);
AsyncLogger::AsyncLogger(Logger *logger, std::string file_name, std::string file_path, int file_size) : logger_(logger), file_name_(std::move(file_name)), file_path_(std::move(file_path)), file_size_(file_size) {
    thread_ = std::jthread(Loop, this);
    sem_.acquire();
}


void *AsyncLogger::Loop(AsyncLogger *async_logger) {
    rpc::AsyncLogger::sem_.release();
    while (true) {
        std::unique_lock lock(async_logger->mutex_);
        async_logger->cond_.wait(lock, [&] { return !async_logger->buffer_.empty() || async_logger->stop_flag_; });
        if (async_logger->stop_flag_) {
            break;
        }
        
        auto vec = std::move(async_logger->buffer_.front());
        async_logger->buffer_.pop();
        lock.unlock();
        
        timeval now{};
        gettimeofday(&now, nullptr);
        struct tm now_time{};
        localtime_r(&now.tv_sec, &now_time);
        const char* format = "%Y-%m-%d";
        char date[64];
        strftime(date, sizeof(date), format, &now_time);
        std::string date_str(date);
        
        if (date_str != async_logger->date_) {
            async_logger->num_ = 0;
            async_logger->reopen_flag = true;
            async_logger->date_ = date_str;
        }
        
        if (async_logger->file_ == nullptr) {
            async_logger->reopen_flag = true;
        }
        
        // 创建日志的输出路径
        std::stringstream ss;
        ss << async_logger->file_path_ << async_logger->file_name_ << "_" << date_str << "_log.";
        std::string log_file_name = ss.str() + std::to_string(async_logger->num_);
        
        // 如果文件不存在，创建文件
        if (async_logger->reopen_flag) {
            // 如果文件存在，关闭文件
            if (async_logger->file_ != nullptr) {
                fclose(async_logger->file_);
                async_logger->file_ = nullptr;
            }
            // 创建文件
            async_logger->file_ = fopen(log_file_name.c_str(), "a+");
            async_logger->reopen_flag = false;
        }
        
        // 如果文件大小超过限制，关闭文件，创建新文件
        if (ftell(async_logger->file_) > async_logger->file_size_) {
            fclose(async_logger->file_);
            async_logger->num_++;
            log_file_name = ss.str() + std::to_string(async_logger->num_);
            async_logger->file_ = fopen(log_file_name.c_str(), "a+");
            async_logger->reopen_flag = false;
        }
        
        for (auto& str : vec) {
            fprintf(async_logger->file_, "%s", str.c_str());
        }
        fflush(async_logger->file_);
        if (async_logger->stop_flag_) {
            return nullptr;
        }
        return nullptr;
    }
}

void AsyncLogger::stop() {
    stop_flag_ = true;
}

void AsyncLogger::flush() {
    if (file_ != nullptr) {
        fflush(file_);
    }
}

void AsyncLogger::pushLogToBuffer(std::vector<std::string> &&vec) {
    std::scoped_lock lock(mutex_);
    buffer_.push(std::move(vec));
    cond_.notify_one();
}

void AsyncLogger::append(std::vector<std::string> &vec) {
    std::scoped_lock lock(mutex_);
    buffer_.push(std::move(vec));
    cond_.notify_one();
}

}