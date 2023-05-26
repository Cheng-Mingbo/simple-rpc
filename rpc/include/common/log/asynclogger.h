//
// Created by cheng on 23-5-25.
//

#ifndef RPC_ASYNCLOGGER_H
#define RPC_ASYNCLOGGER_H
#include <queue>
#include <string>
#include <semaphore>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>


namespace rpc {
class Logger;
class AsyncLogger {
  public:
    using s_AsyncLoggerPtr = std::shared_ptr<AsyncLogger>;
    explicit AsyncLogger(Logger* logger, std::string  file_name, std::string  file_path, int file_size);
    
    void stop();
    
    void flush();
    
    void pushLogToBuffer(std::vector<std::string>&& vec);
    
    void append(std::vector<std::string>& vec);
    
  public:
    static void* Loop(AsyncLogger* async_logger);
  
  private:
    Logger* logger_;
    std::queue<std::vector<std::string>> buffer_;
    FILE* file_{nullptr};
    
    std::string file_name_;
    std::string file_path_;
    int file_size_{0};
    
    std::condition_variable cond_;
    std::mutex mutex_;
    std::jthread thread_;
    
    std::string date_;
    static std::binary_semaphore sem_;
    bool reopen_flag {false};
    int num_{0};
    std::atomic<bool> stop_flag_{false};
};
}
#endif //RPC_ASYNCLOGGER_H
