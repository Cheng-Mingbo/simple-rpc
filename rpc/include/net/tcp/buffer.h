//
// Created by cheng on 23-5-28.
//

#ifndef RPC_BUFFER_H
#define RPC_BUFFER_H
#include <vector>
#include <memory>

namespace rpc {
class TcpBuffer {
  public:
    using s_ptr = std::shared_ptr<TcpBuffer>;
    explicit TcpBuffer(int size);
    ~TcpBuffer();
    
    int readableBytes() const;
    int writableBytes() const;
    
    int readIndex() const;
    int writeIndex() const;
    
    void writeToBuffer(const char* data, int len);
    bool readFromBuffer(std::vector<char>& re, int len);
    bool readFromBuffer(char& re);
    bool readFromBuffer(int32_t& re);
    bool readFromBuffer(std::string& re, int len);
    void adjustBuffer();
    
    void moveReadIndex(int len);
    void moveWriteIndex(int len);
    
    auto getWritePtr() { return buffer_.data() + write_index_; }
    auto getReadPtr() { return buffer_.data() + read_index_; }
    
    std::size_t getSize() const { return buffer_.size(); }
    
    void resize(int size);
    
  private:
    int read_index_ {0};
    int write_index_ {0};
    int capacity_ {0};
    
    std::vector<char> buffer_;
};
}
#endif //RPC_BUFFER_H
