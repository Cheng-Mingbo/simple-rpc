//
// Created by cheng on 23-5-27.
//

#ifndef RPC_WAKEUP_CHANNEL_H
#define RPC_WAKEUP_CHANNEL_H
#include "channel.h"

namespace rpc {
class WakeupChannel : public Channel {
  public:
    explicit WakeupChannel(int fd) : Channel(fd) {};
    ~WakeupChannel() override = default;
    
    void wakeup();
};
}
#endif //RPC_WAKEUP_CHANNEL_H
