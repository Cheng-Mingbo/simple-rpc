//
// Created by cheng on 23-5-27.
//

#include "timer_channel.h"
#include "util.h"

namespace rpc {
void TimerChannel::resetArriveTime() {
    arrive_time_ = getNowTime() + interval_;
}
}