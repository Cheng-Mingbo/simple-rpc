//
// Created by cheng on 23-5-29.
//

#ifndef RPC_ABSTRACT_CODER_H
#define RPC_ABSTRACT_CODER_H
#include <vector>
#include "buffer.h"
#include "abstract_protocol.h"
namespace rpc {
class AbstractCoder {
  public:
    virtual void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr output_buffer) = 0;
    virtual void decode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr input_buffer) = 0;
    virtual ~AbstractCoder() = default;
};
}
#endif //RPC_ABSTRACT_CODER_H
