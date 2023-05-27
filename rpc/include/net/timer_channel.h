//
// Created by cheng on 23-5-27.
//

#ifndef RPC_TIMER_CHANNEL_H
#define RPC_TIMER_CHANNEL_H
#include <functional>
#include <memory>

namespace rpc {
class TimerChannel {
  public:
    using s_ptr = std::shared_ptr<TimerChannel>;
    explicit TimerChannel(int interval, bool repeat, std::function<void()> callback) :
        interval_(interval), repeat_(repeat), task_(std::move(callback)) { resetArriveTime(); };
    ~TimerChannel() = default;
    
    int64_t getArriveTime() const { return arrive_time_; }
    
    void setCancel(bool cancel) { cancel_ = cancel; }
    bool isCancel() const { return cancel_; }
    
    bool isRepeat() const { return repeat_; }
    
    std::function<void()> getTask() { return task_; }
    
    void resetArriveTime();
    
  private:
    int64_t interval_;
    int64_t arrive_time_;
    bool repeat_ {false};
    bool cancel_ {false};
    
    std::function<void()> task_;
};

}

#endif //RPC_TIMER_CHANNEL_H
