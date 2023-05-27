//
// Created by cheng on 23-5-27.
//

#ifndef RPC_TIMER_H
#define RPC_TIMER_H
#include "channel.h"
#include "timer_channel.h"
#include <mutex>
#include <map>

namespace rpc {
class Timer : public Channel {
  public:
    Timer();
    ~Timer() override = default;
    
    void addTimer(TimerChannel::s_ptr timer_channel);
    void removeTimer(TimerChannel::s_ptr timer_channel);
    
    void onTimer();
    
  private:
    void resetArriveTime();
    
  private:
    std::multimap<int64_t, TimerChannel::s_ptr> timer_channels_;
    std::mutex mutex_;
    
};
}
#endif //RPC_TIMER_H
