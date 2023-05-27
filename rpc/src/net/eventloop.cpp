//
// Created by cheng on 23-5-27.
//
#include <sys/eventfd.h>
#include <cstring>
#include <utility>
#include "eventloop.h"
#include "logger.h"
#include "util.h"

#define ADD_EPOLL_EVENT() \
    auto it = listen_fds_.find(channel->getFd());\
    int op = EPOLL_CTL_ADD;\
    if (it != listen_fds_.end()) {\
    op = EPOLL_CTL_MOD;\
    }\
    epoll_event tmp = channel->getEvents();\
    if (epoll_ctl(epoll_fd_, op, channel->getFd(), &tmp) < 0) {\
    LOG_ERROR("EventLoop::addEpollEvent() error: %d, info[%s]", errno, strerror(errno));\
    }\
    listen_fds_.insert(channel->getFd());\
    LOG_DEBUG("EventLoop::addEpollEvent() success: fd: %d", channel->getFd());                         \
    
#define DEL_EPOLL_EVENT() \
    auto it = listen_fds_.find(channel->getFd());\
    int op = EPOLL_CTL_DEL;\
    if (it != listen_fds_.end()) {\
        return;\
    }\
    epoll_event tmp = channel->getEvents();\
    if (epoll_ctl(epoll_fd_, op, channel->getFd(), &tmp) < 0) {\
    LOG_ERROR("EventLoop::removeEpollEvent() error: %d, info[%s]", errno, strerror(errno));\
    }\
    listen_fds_.erase(channel->getFd());\
    LOG_DEBUG("EventLoop::removeEpollEvent() success: fd: %d", channel->getFd());\
    
    
namespace rpc {

static thread_local EventLoop* t_loop = nullptr;
static int g_max_timeout = 10000;
static int g_max_event_num = 1024;

EventLoop::EventLoop() {
   if (t_loop != nullptr) {
       LOG_ERROR("EventLoop::EventLoop() error: already have a EventLoop in this thread");
       exit(1);
   }
   tid_ = getThreadId();
   epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
         LOG_ERROR("EventLoop::EventLoop() error: %d, info[%s]", errno, strerror(errno));
         exit(1);
    }
    initWakeupChannel();
    initTimer();
    
    LOG_INFO("Success create EventLoop in thread: %d", tid_);
    t_loop = this;
}

EventLoop::~EventLoop() {
    ::close(epoll_fd_);
    if (wakeup_channel_) {
        wakeup_channel_->close();
        delete wakeup_channel_;
        wakeup_channel_ = nullptr;
    }
    if (timer_) {
        timer_->close();
        delete timer_;
        timer_ = nullptr;
    }
}

void EventLoop::initTimer() {
    timer_ = new Timer();
    addEpollEvent(timer_);
}

void EventLoop::addTimer(TimerChannel::s_ptr timer_channel) {
    timer_->addTimer(std::move(timer_channel));
    LOG_INFO("Success add timer in EventLoop in thread: %d", tid_);
}

void EventLoop::initWakeupChannel() {
    wakeup_fd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (wakeup_fd_ < 0) {
        LOG_ERROR("EventLoop::initWakeupChannel() error: %d, info[%s]", errno, strerror(errno));
        exit(1);
    }
    wakeup_channel_ = new WakeupChannel(wakeup_fd_);
    wakeup_channel_->listen(Channel::TriggerEvent::kReadEvent, [this]() {
        uint64_t one = 1;
        ssize_t n = read(wakeup_fd_, &one, sizeof(one));
        if (n != sizeof(one)) {
            LOG_ERROR("EventLoop::initWakeupChannel() error: %d, info[%s]", errno, strerror(errno));
        }
        LOG_INFO("Success wakeup EventLoop in thread: %d", tid_);
    });
    addEpollEvent(wakeup_channel_);
}

void EventLoop::wakeup() {
    wakeup_channel_->wakeup();
}

void EventLoop::stop() {
    stop_ = true;
    wakeup();
}

void EventLoop::loop() {
    stop_ = false;
    while (!stop_) {
        std::unique_lock<std::mutex> lock(mutex_);
        std::queue<std::function<void()>> tmp_tasks;
        pending_tasks_.swap(tmp_tasks);
        lock.unlock();
        
        while (!tmp_tasks.empty()) {
            auto cb = std::move(tmp_tasks.front());
            tmp_tasks.pop();
            if (cb) {
                cb();
            }
        }
        
        int timeout = g_max_timeout;
        std::vector<epoll_event> events(g_max_event_num);
        int num = epoll_wait(epoll_fd_, events.data(), g_max_event_num, timeout);
        LOG_DEBUG("EventLoop::loop() epoll_wait return: %d", num);
        if (num < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR("EventLoop::loop() error: %d, info[%s]", errno, strerror(errno));
            exit(1);
        } else if (num == 0) {
            // timeout
            continue;
        } else {
            for (int i = 0; i < num; i++) {
                epoll_event trigger_event = events[i];
                Channel* channel = static_cast<Channel*>(trigger_event.data.ptr);
                if (channel == nullptr) {
                    LOG_ERROR("EventLoop::loop() error: channel is nullptr");
                    continue;
                }
                int event = (int)trigger_event.events;
                if (trigger_event.events & EPOLLIN) {
                    addTask(channel->handler(Channel::TriggerEvent::kReadEvent));
                }
                if (trigger_event.events & EPOLLOUT) {
                    addTask(channel->handler(Channel::TriggerEvent::kWriteEvent));
                }
                if (trigger_event.events & EPOLLERR) {
                    removeEpollEvent(channel);
                    if (channel->handler(Channel::TriggerEvent::kErrorEvent) != nullptr) {
                        addTask(channel->handler(Channel::TriggerEvent::kErrorEvent));
                    }
                }
            }
        }
    }
}

void EventLoop::addEpollEvent(Channel *channel) {
    if (isInLoopThread()) {
        ADD_EPOLL_EVENT();
    } else {
        auto cb = [this, &channel]() {
            ADD_EPOLL_EVENT();
        };
        addTask(cb, true);
    }
}

void EventLoop::removeEpollEvent(rpc::Channel *channel) {
    if (isInLoopThread()) {
        DEL_EPOLL_EVENT();
    } else {
        auto cb = [this, &channel]() {
            DEL_EPOLL_EVENT();
        };
        addTask(cb, true);
    }
}

void EventLoop::addTask(std::function<void()> task, bool wakup) {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_tasks_.push(std::move(task));
    lock.unlock();
    if (wakup) {
        wakeup();
    }
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    if (t_loop) {
       return t_loop;
    }
    t_loop = new EventLoop();
    return t_loop;
}

bool EventLoop::isLooping() {
    return !stop_;
}

bool EventLoop::isInLoopThread() const {
    return tid_ == getThreadId();
}

}

