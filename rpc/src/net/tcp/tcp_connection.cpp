//
// Created by cheng on 23-5-28.
//

#include "tcp_connection.h"
#include "channel_group.h"
#include "logger.h"

#include <utility>

namespace rpc {

TcpConnection::TcpConnection(EventLoop *loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr,
                             NetAddr::s_ptr local_addr, TcpConnectionType type) : loop_(loop),
                                                                                    peer_addr_(std::move(peer_addr)),
                                                                                    local_addr_(std::move(local_addr)),
                                                                                    type_(type),
                                                                                    state_(TcpConnectionState::Connected)
                                                                                     {
    input_buffer_ = std::make_shared<TcpBuffer>(buffer_size);
    output_buffer_ = std::make_shared<TcpBuffer>(buffer_size);
    
    channel_ = ChannelGroup::GetChannelGroup()->getChannel(fd);
    channel_->setNonBlock();
    
    // TODO: init coder
    // ...
    
    if (type_ == TcpConnectionType::Server) {
        listenRead();
    }
    
}

TcpConnection::~TcpConnection() {
    // TODO: delete coder
    // ...
}

void TcpConnection::onRead() {
    if (state_ == TcpConnectionState::Closed) {
        LOG_ERROR("TcpConnection::onRead() error, state is closed");
        return;
    }
    bool is_read_all = false;
    bool is_close = false;
    
    while (!is_read_all) {
        if (input_buffer_->writableBytes() == 0) {
            input_buffer_->resize((int)input_buffer_->getSize() * 2);
        }
        int read_count = input_buffer_->writableBytes();
        auto write_ptr = input_buffer_->getWritePtr();
        ssize_t n = ::read(channel_->getFd(), write_ptr, read_count);
        if (n > 0) {
            input_buffer_->moveWriteIndex(n);
            if (n == read_count) {
                continue;
            } else if (n < read_count) {
                is_read_all = true;
                break;
            }
        } else if (n == 0) {
            is_close = true;
            break;
        } else if (n == -1 && errno == EAGAIN) {
            is_read_all = true;
            break;
        }
    }
    
    if (is_close) {
        LOG_INFO("TcpConnection::onRead() close");
        clear();
        return;
    }
    
    if (!is_read_all) {
        LOG_ERROR("TcpConnection::onRead() error");
    }
    
    excute();
}

void TcpConnection::excute() {
    // TODO: excute
    
    // 将input_buffer_中的数据全部放入output_buffer_中
    std::vector<char> data;
    input_buffer_->readFromBuffer(data, input_buffer_->readableBytes());
    std::string str(data.begin(), data.end());
    output_buffer_->writeToBuffer(str.c_str(), str.size());
    listenWrite();
}

void TcpConnection::onWrite() {
    if (state_ != TcpConnectionState::Connected) {
        LOG_ERROR("TcpConnection::onWrite() error, state is not connected");
        return;
    }
    
    // TODO: implement Client's write
    // ...
    
    bool is_write_all = false;
    while (true) {
        if (output_buffer_->readableBytes() == 0) {
            LOG_DEBUG("No data to write");
            is_write_all = true;
            break;
        }
        int write_size = output_buffer_->readableBytes();
        auto read_ptr = output_buffer_->getReadPtr();
        int n = ::write(channel_->getFd(), read_ptr, write_size);
        
        if (n > 0) {
            LOG_INFO("write %d bytes, msg: %s", n, read_ptr);
            output_buffer_->moveReadIndex(n);
            if (n == write_size) {
                continue;
            } else if (n < write_size) {
                is_write_all = true;
                break;
            }
        } else if (n == -1 && errno == EAGAIN) {
            break;
        } else {
            LOG_ERROR("TcpConnection::onWrite() error");
            break;
        }
    }
    if (is_write_all) {
        channel_->cancel(Channel::TriggerEvent::kWriteEvent);
        loop_->addEpollEvent(channel_);
    }
    
    // TODO: implement Client's write
}

void TcpConnection::clear() {
    if (state_ == TcpConnectionState::Closed) {
        return;
    }
    channel_->cancel(Channel::TriggerEvent::kReadEvent);
    channel_->cancel(Channel::TriggerEvent::kWriteEvent);
    // loop_->addEpollEvent(channel_);
    loop_->removeEpollEvent(channel_);
    state_ = TcpConnectionState::Closed;
}

void TcpConnection::shutdown() {
    if(state_ == TcpConnectionState::Closed || state_ == TcpConnectionState::NotConnected) {
        return;
    }
    state_ = TcpConnectionState::HalfClosing;
    ::shutdown(channel_->getFd(), SHUT_RDWR);
}

void TcpConnection::listenWrite() {
    channel_->listen(Channel::TriggerEvent::kWriteEvent, std::bind(&TcpConnection::onWrite, this));
    loop_->addEpollEvent(channel_);
}

void TcpConnection::listenRead() {
    channel_->listen(Channel::TriggerEvent::kReadEvent, std::bind(&TcpConnection::onRead, this));
    loop_->addEpollEvent(channel_);
}

void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr protocol, std::function<void(AbstractProtocol::s_ptr)> done) {
    write_dons_.emplace_back(std::move(protocol), std::move(done));
}

void TcpConnection::pushReadMessage(const std::string &msg_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    read_dons_.insert(std::make_pair(msg_id, std::move(done)));
}
}