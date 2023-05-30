//
// Created by cheng on 23-5-27.
//

#ifndef RPC_EVENTLOOP_H
#define RPC_EVENTLOOP_H
#include <pthread.h>
#include <sys/epoll.h>
#include <set>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>

#include "wakeup_channel.h"
#include "timer.h"

namespace rpc {
class EventLoop {
  public:
    EventLoop();
    ~EventLoop();
    
    void loop();
    void wakeup();
    
    void stop();
    
    void addEpollEvent(Channel* channel);
    void removeEpollEvent(Channel* channel);
    
    bool isInLoopThread() const;
    void addTask(std::function<void()> task, bool wakup = false);
    void addTimer(TimerChannel::s_ptr timer_channel);
    
    bool isLooping();
    
  public:
    static EventLoop* getEventLoopOfCurrentThread();
    
  private:
    void initWakeupChannel();
    void initTimer();
    
  private:
    pid_t tid_;
    std::set<int> listen_fds_;
    int epoll_fd_;
    std::mutex  mutex_;
    std::atomic<bool> stop_ {true };
    int wakeup_fd_;
    WakeupChannel* wakeup_channel_ {nullptr};
    Timer* timer_ {nullptr};
    std::queue<std::function<void()>> pending_tasks_;
};
}

#endif //RPC_EVENTLOOP_H
