//
// Created by cheng on 23-5-27.
//

#ifndef RPC_IO_THREAD_H
#define RPC_IO_THREAD_H
#include <pthread.h>
#include <semaphore>
#include <thread>]
#include "eventloop.h"

namespace rpc {
class IOThread {
  public:
    IOThread();
    ~IOThread();
    
    void start();
    void join();
    EventLoop* getEventLoop() { return event_loop_; }
    
    static void threadFunc(IOThread* io_thread);
    
  private:
    pid_t tid_{};
    std::jthread thread_;
    EventLoop* event_loop_{};
    
    std::binary_semaphore init_sem_;
    std::binary_semaphore start_sem_;
};

}
#endif //RPC_IO_THREAD_H
