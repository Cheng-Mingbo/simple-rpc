//
// Created by cheng on 23-5-28.
//

#include <netinet/in.h>
#include "buffer.h"

namespace rpc {
TcpBuffer::TcpBuffer(int size) : capacity_(size) {
    buffer_.resize(size);
}

TcpBuffer::~TcpBuffer() = default;

int TcpBuffer::readableBytes() const {
    return write_index_ - read_index_;
}

int TcpBuffer::writableBytes() const {
    return capacity_ - write_index_;
}

int TcpBuffer::readIndex() const {
    return read_index_;
}

int TcpBuffer::writeIndex() const {
    return write_index_;
}

void TcpBuffer::writeToBuffer(const char *data, int len) {
    if (len > writableBytes()) {
        resize(2 * (len + readableBytes()));
    }
    std::copy(data, data + len, buffer_.begin() + write_index_);
    write_index_ += len;
}

void TcpBuffer::resize(int size) {
    std::vector<char> tmp(size);
    int count = std::min(readableBytes(), size);
    std::copy(buffer_.begin() + read_index_, buffer_.begin() + write_index_, tmp.begin());
    buffer_ = std::move(tmp);
    read_index_ = 0;
    write_index_ = read_index_ + count;
    capacity_ = size;
}

bool TcpBuffer::readFromBuffer(std::vector<char> &re, int len) {
    if (readableBytes() == 0) {
        return false;
    }
    int read_size = std::min(readableBytes(), len);
    std::vector<char> tmp(read_size);
    std::copy(buffer_.begin() + read_index_, buffer_.begin() + read_index_ + read_size, tmp.begin());
    re = std::move(tmp);
    read_index_ += read_size;
    adjustBuffer();
    return true;
}

void TcpBuffer::adjustBuffer() {
    if (read_index_ < buffer_.size() / 3) {
        return;
    }
    std::vector<char> tmp(buffer_.size());
    int count = readableBytes();
    std::copy(buffer_.begin() + read_index_, buffer_.begin() + write_index_, tmp.begin());
    buffer_ = std::move(tmp);
    read_index_ = 0;
    write_index_ = read_index_ + count;
}

void TcpBuffer::moveReadIndex(int len) {
    if (read_index_ + len > write_index_ || read_index_ + len > capacity_) {
        return;
    }
    read_index_ += len;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int len) {
    if (write_index_ + len > capacity_) {
        adjustBuffer();
    }
    write_index_ += len;
    
}

bool TcpBuffer::readFromBuffer(char &re) {
    if (readableBytes() == 0) {
        return false;
    }
    re = buffer_[read_index_];
    read_index_++;
    adjustBuffer();
    return true;
}

bool TcpBuffer::readFromBuffer(int32_t &re) {
    if (readableBytes() < 4) {
        return false;
    }
    re = *(int32_t *) (buffer_.data() + read_index_);
    re = ::ntohl(re);
    read_index_ += 4;
    adjustBuffer();
    return true;
}

bool TcpBuffer::readFromBuffer(std::string &re, int len) {
    if (readableBytes() < len) {
        return false;
    }
    re = std::string(buffer_.begin() + read_index_, buffer_.begin() + read_index_ + len);
    read_index_ += len;
    adjustBuffer();
    return true;
}
}