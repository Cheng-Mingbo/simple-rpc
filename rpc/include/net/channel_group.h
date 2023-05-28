//
// Created by cheng on 23-5-28.
//

#ifndef RPC_CHANNEL_GROUP_H
#define RPC_CHANNEL_GROUP_H
#include <vector>
#include <mutex>
#include "channel.h"

namespace rpc {
class ChannelGroup {
  public:
    ChannelGroup(int size);
    ~ChannelGroup();
    
    Channel* getChannel(int fd);
    
  public:
    static ChannelGroup* GetChannelGroup();
    
  private:
    int size_{0};
    std::vector<Channel*> channels_;
    std::mutex mutex_;
};
}
#endif //RPC_CHANNEL_GROUP_H
