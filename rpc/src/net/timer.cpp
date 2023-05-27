//
// Created by cheng on 23-5-27.
//

#include <sys/timerfd.h>
#include <cstring>
#include "timer.h"
#include "logger.h"
#include "util.h"


namespace rpc {
Timer::Timer() : Channel() {
    fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    LOG_DEBUG("Timer::Timer() fd: %d", fd_);
    
    listen(Channel::TriggerEvent::kReadEvent, std::bind(&Timer::onTimer, this));
}

void Timer::onTimer() {
    LOG_DEBUG("Timer::onTimer()");
    uint64_t one = 1;
    ssize_t n = read(fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("Timer::onTimer() error: %d, info[%s]", errno, strerror(errno));
    }
    
    int64_t now = getNowTime();
    std::vector<TimerChannel::s_ptr> tmp_timer_channels;
    std::vector<std::pair<int64_t, std::function<void()>>> tasks;
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        auto it = timer_channels_.begin();
        while (it != timer_channels_.end()) {
            if (it->first <= now) {
                if (!it->second->isCancel()) {
                    tmp_timer_channels.push_back(it->second);
                    tasks.emplace_back(it->second->getArriveTime(), it->second->getTask());
                }
            } else {
                break;
            }
            it++;
        }
        
        timer_channels_.erase(timer_channels_.begin(), it);
    }
    
    for (auto& timer_channel : tmp_timer_channels) {
        if (timer_channel->isRepeat()) {
            timer_channel->resetArriveTime();
            addTimer(timer_channel);
        }
    }
    resetArriveTime();
    for (auto& task : tasks) {
        if (task.second) {
            task.second();
        }
    }
    
}

void Timer::resetArriveTime() {
    std::multimap<int64_t, TimerChannel::s_ptr> tmp;
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        tmp = timer_channels_;
    }
    if (tmp.size() == 0) {
        return;
    }
    auto now = getNowTime();
    auto it = tmp.begin();
    int64_t interval = 0;
    if (it->second->getArriveTime() > now) {
        interval = it->second->getArriveTime() - now;
    } else {
        interval = 100;
    }
    
    timespec ts{};
    bzero(&ts, sizeof(ts));
    ts.tv_sec = interval / 1000;
    ts.tv_nsec = (interval % 1000) * 1000000;
    itimerspec new_value{};
    bzero(&new_value, sizeof(new_value));
    new_value.it_value = ts;
    int ret = timerfd_settime(fd_, 0, &new_value, nullptr);
    if (ret < 0) {
        LOG_ERROR("Timer::resetArriveTime() error: %d, info[%s]", errno, strerror(errno));
    }
    
}

void Timer::addTimer(TimerChannel::s_ptr timer_channel) {
    bool is_reset_timerfd = false;
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        if (timer_channels_.empty()) {
            is_reset_timerfd = true;
        } else {
            auto it = timer_channels_.begin();
            if (it->first > timer_channel->getArriveTime()) {
                is_reset_timerfd = true;
            }
        }
        timer_channels_.emplace(timer_channel->getArriveTime(), timer_channel);
    }
    if (is_reset_timerfd) {
        resetArriveTime();
    }
}

void Timer::removeTimer(TimerChannel::s_ptr timer_channel) {
    timer_channel->setCancel(true);
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        auto begin = timer_channels_.lower_bound(timer_channel->getArriveTime());
        auto end = timer_channels_.upper_bound(timer_channel->getArriveTime());
        
        auto it = begin;
        while (it != end) {
            if (it->second == timer_channel) {
                timer_channels_.erase(it);
                break;
            }
        }
    }
}


}