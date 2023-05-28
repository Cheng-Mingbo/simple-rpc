//
// Created by cheng on 23-5-28.
//

#include "channel_group.h"


namespace rpc {
static ChannelGroup *g_channel_group = nullptr;

ChannelGroup::ChannelGroup(int size) : size_(size) {
    for (int i = 0; i < size_; ++i) {
        channels_.push_back(new Channel(i));
    }
}

ChannelGroup *ChannelGroup::GetChannelGroup() {
    if (g_channel_group == nullptr) {
        std::once_flag flag;
        std::call_once(flag, []() {
            g_channel_group = new ChannelGroup(1024);
        });
    }
    return g_channel_group;
}

ChannelGroup::~ChannelGroup() {
    for (auto &channel: channels_) {
        delete channel;
    }
}

Channel *ChannelGroup::getChannel(int fd) {
    std::scoped_lock<std::mutex> lock(mutex_);
    if (fd < channels_.size()) {
        return channels_[fd];
    }
    
    int new_size = fd * 1.5;
    for (int i = channels_.size(); i < new_size; ++i) {
        channels_.push_back(new Channel(i));
    }
    return channels_[fd];
}

}