//
// Created by cheng on 23-5-29.
//

#ifndef RPC_TINYPB_CODER_H
#define RPC_TINYPB_CODER_H
#include "abstract_coder.h"
#include "tinypb_protocol.h"
#include <memory>

namespace rpc {
class TinyPBCoder : public AbstractCoder {
  public:
    using s_ptr = std::shared_ptr<TinyPBCoder>;
    
    enum class DecodeState {
        SEARCHING_START,
        READING_PACKAGE_LEN,
        READING_MSG_ID_LEN,
        READING_MSG_ID,
        READING_METHOD_NAME_LEN,
        READING_METHOD_NAME,
        READING_ERR_CODE,
        READING_ERR_MSG_LEN,
        READING_ERR_MSG,
        READING_PB_DATA,
        READING_CHECK_SUM,
        SEARCHING_END,
    };
    TinyPBCoder() : message_(std::make_shared<TinyPBProtocol>()) {}
    ~TinyPBCoder() override = default;
    
    void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr output_buffer) override;
    void decode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr input_buffer) override;
    
  private:
    DecodeState decode_state_ {DecodeState::SEARCHING_START};
    TinyPBProtocol::s_ptr message_;
    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> &message,  int& len);
};
}
#endif //RPC_TINYPB_CODER_H
