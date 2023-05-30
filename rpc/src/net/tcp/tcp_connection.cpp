//
// Created by cheng on 23-5-28.
//

#include "tcp_connection.h"
#include "channel_group.h"
#include "logger.h"
#include "tinypb_coder.h"
#include "tinypb_protocol.h"
#include "rpc_dispatcher.h"

#include <utility>
#include <cstring>

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
    
    coder_ = new TinyPBCoder();
    
    if (type_ == TcpConnectionType::Server) {
        listenRead();
    }
    
}

TcpConnection::~TcpConnection() {
    if (coder_) {
        delete coder_;
        coder_ = nullptr;
    }
}

void TcpConnection::onRead() {
    if (state_ == TcpConnectionState::Closed) {
        LOG_ERROR("TcpConnection::onRead() error, state is closed");
        return;
    }
    bool is_read_all = false;
    bool is_close = false;
    
    char buffer[1024];
    bzero(buffer, sizeof(buffer));
    
    while (!is_read_all) {
        ssize_t n = ::read(channel_->getFd(), buffer, sizeof(buffer));
        LOG_DEBUG("TcpConnection::onRead() read %d bytes", n);
        if (n > 0) {
            input_buffer_->writeToBuffer(buffer, n);
            bzero(buffer, sizeof(buffer));
        } else if (n == 0) {
            is_close = true;
            break;
        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
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
    if (type_ == TcpConnectionType::Server) {
        std::vector<AbstractProtocol::s_ptr> messages;
        std::vector<AbstractProtocol::s_ptr> responses;
        coder_->decode(messages, input_buffer_);
        for (auto & message : messages) {
            LOG_INFO("Success receive request [%s] from client [%s]", message->msg_id_.c_str(), peer_addr_->toString().c_str());
            TinyPBProtocol::s_ptr reply = std::make_shared<TinyPBProtocol>();
            RpcDispatcher::GetRpcDispatcher()->dispatch(message, reply, this);
            LOG_DEBUG("Success dispatch request [%s] from client [%s], reponse is %s", message->msg_id_.c_str(), peer_addr_->toString().c_str(), reply->msg_id_.c_str());
            responses.push_back(reply);
            LOG_DEBUG("Success add response [%s] to response list", reply->toString().c_str());
        }
        coder_->encode(responses, output_buffer_);
        listenWrite();
    } else {
       std::vector<AbstractProtocol::s_ptr> messages;
         coder_->decode(messages, input_buffer_);
            for (auto & message : messages) {
                LOG_INFO("Success receive response [%s] from server [%s]", message->msg_id_.c_str(), peer_addr_->toString().c_str());
                auto it = read_dons_.find(message->msg_id_);
                if (it != read_dons_.end()) {
                    it->second(message);
                    read_dons_.erase(it);
                }
            }
    }
}

void TcpConnection::onWrite() {
    if (state_ != TcpConnectionState::Connected) {
        LOG_ERROR("TcpConnection::onWrite() error, state is not connected");
        return;
    }
    
    if (type_ == TcpConnectionType::Client) {
        std::vector<AbstractProtocol::s_ptr> messages;
        
        for (auto & write_don : write_dons_) {
            messages.push_back(write_don.first);
        }
        coder_->encode(messages, output_buffer_);
    }
    
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
        LOG_DEBUG("TcpConnection::onWrite() write %d bytes", n);
        if (n > 0) {
            output_buffer_->moveReadIndex(n);
            if (output_buffer_->readableBytes() == 0) {
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
    
    if (type_ == TcpConnectionType::Client) {
        for (auto & write_don : write_dons_) {
            write_don.second(write_don.first);
        }
        write_dons_.clear();
    }
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