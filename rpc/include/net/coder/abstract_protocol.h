//
// Created by cheng on 23-5-28.
//

#ifndef RPC_ABSTRACT_PROTOCOL_H
#define RPC_ABSTRACT_PROTOCOL_H
#include <memory>

namespace rpc {
  class AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {
    public:
      using s_ptr = std::shared_ptr<AbstractProtocol>;
        virtual ~AbstractProtocol() = default;
        
    public:
      std::string msg_id;
  };
}
#endif //RPC_ABSTRACT_PROTOCOL_H
