//
// Created by cheng on 23-5-29.
//

#ifndef RPC_TINYPB_PROTOCOL_H
#define RPC_TINYPB_PROTOCOL_H
#include "abstract_protocol.h"

namespace rpc {
struct TinyPBProtocol : public AbstractProtocol {
  public:
    using s_ptr = std::shared_ptr<TinyPBProtocol>;
    TinyPBProtocol() = default;
    ~TinyPBProtocol() override = default;
    
    std::string toString();
    
  public:
    static char PB_START_FLAG;
    static char PB_END_FLAG;
    
  public:
    int32_t package_len_ {0};
    int32_t msg_id_len_ {0};
    
    int32_t method_name_len_ {0};
    std::string method_name_;
    int32_t err_code_ {0};
    int32_t err_msg_len_ {0};
    std::string err_msg_;
    std::string pb_data_;
    int32_t check_sum_ {0};
    
    bool parse_success_ {false};
};


}
#endif //RPC_TINYPB_PROTOCOL_H
