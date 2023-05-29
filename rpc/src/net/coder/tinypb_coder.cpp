//
// Created by cheng on 23-5-29.
//

#include <netinet/in.h>
#include <cstring>
#include "tinypb_coder.h"
#include "tinypb_protocol.h"

namespace rpc {

void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr output_buffer) {
    for (auto &message : messages) {
        auto tiny_pb_message = std::dynamic_pointer_cast<TinyPBProtocol>(message);
        int len = 0;
        auto data = encodeTinyPB(tiny_pb_message, len);
        if (data != nullptr && len != 0) {
            output_buffer->writeToBuffer(data, len);
        }
        if (data != nullptr) {
            delete[] data;
            data = nullptr;
        }
        
    }
}

void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr input_buffer) {
    while (true) {
        switch (decode_state_) {
            case DecodeState::SEARCHING_START:
                char ch;
                if (!input_buffer->readFromBuffer(ch)) {
                    return;
                } else if (ch == TinyPBProtocol::PB_START_FLAG) {
                    decode_state_ = DecodeState::READING_PACKAGE_LEN;
                }
                break;
            case DecodeState::READING_PACKAGE_LEN:
                if (!input_buffer->readFromBuffer(message_->package_len_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_MSG_ID_LEN;
                break;
            case DecodeState::READING_MSG_ID_LEN:
                if (!input_buffer->readFromBuffer(message_->msg_id_len_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_MSG_ID;
                break;
            case DecodeState::READING_MSG_ID:
                if (!input_buffer->readFromBuffer(message_->msg_id_, message_->msg_id_len_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_METHOD_NAME_LEN;
                break;
            case DecodeState::READING_METHOD_NAME_LEN:
                if (!input_buffer->readFromBuffer(message_->method_name_len_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_METHOD_NAME;
                break;
            case DecodeState::READING_METHOD_NAME:
                if (!input_buffer->readFromBuffer(message_->method_name_, message_->method_name_len_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_ERR_CODE;
                break;
            case DecodeState::READING_ERR_CODE:
                if (!input_buffer->readFromBuffer(message_->err_code_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_ERR_MSG_LEN;
                break;
            case DecodeState::READING_ERR_MSG_LEN:
                if (!input_buffer->readFromBuffer(message_->err_msg_len_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_ERR_MSG;
                break;
            case DecodeState::READING_ERR_MSG:
                if (!input_buffer->readFromBuffer(message_->err_msg_, message_->err_msg_len_)) {
                    return;
                }
                decode_state_ = DecodeState::READING_PB_DATA;
                break;
            case DecodeState::READING_PB_DATA:
                if (!input_buffer->readFromBuffer(message_->pb_data_, message_->package_len_ - message_->msg_id_len_ - message_->method_name_len_ - message_->err_msg_len_ - 2 - 24)) {
                    return;
                }
                decode_state_ = DecodeState::READING_CHECK_SUM;
                break;
            case DecodeState::READING_CHECK_SUM:
                if (!input_buffer->readFromBuffer(message_->check_sum_)) {
                    return;
                }
                decode_state_ = DecodeState::SEARCHING_END;
                break;
            case DecodeState::SEARCHING_END:
                if (!input_buffer->readFromBuffer(ch)) {
                    return;
                } else if (ch == TinyPBProtocol::PB_END_FLAG) {
                    message_->parse_success_ = true;
                    messages.push_back(message_);
                    message_ = std::make_shared<TinyPBProtocol>();
                    decode_state_ = DecodeState::SEARCHING_START;
                } else {
                    decode_state_ = DecodeState::SEARCHING_START;
                }
                break;
        }
    }
}

const char *TinyPBCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocol> &message, int& len) {
    if (message->msg_id_.empty()) {
        message->msg_id_len_ = 123456;
    }
    int pk_len = 2 + 24 + message->msg_id_.length() + message->method_name_.length() + message->err_msg_.length() + message->pb_data_.length();
    char* buf = reinterpret_cast<char*>(malloc(pk_len));
    char* tmp = buf;
    *tmp = TinyPBProtocol::PB_START_FLAG;
    tmp += 1;
    
    int32_t package_len = ::htonl(pk_len);
    memcpy(tmp, &package_len, sizeof(int32_t));
    tmp += sizeof(package_len);
    
    message->msg_id_len_ = message->msg_id_.length();
    int32_t msg_id_len = ::htonl(message->msg_id_len_);
    memcpy(tmp, &msg_id_len, sizeof(int32_t));
    tmp += sizeof(msg_id_len);
    
    if (!message->msg_id_.empty()) {
        memcpy(tmp, message->msg_id_.c_str(), message->msg_id_len_);
        tmp += message->msg_id_len_;
    }
    
    message->method_name_len_ = message->method_name_.length();
    int32_t method_name_len = ::htonl(message->method_name_len_);
    memcpy(tmp, &method_name_len, sizeof(int32_t));
    tmp += sizeof(method_name_len);
    
    if (!message->method_name_.empty()) {
        memcpy(tmp, message->method_name_.c_str(), message->method_name_len_);
        tmp += message->method_name_len_;
    }
    
    message->err_code_ = 0;
    int32_t err_code = ::htonl(message->err_code_);
    memcpy(tmp, &err_code, sizeof(int32_t));
    tmp += sizeof(err_code);
    
    message->err_msg_len_ = message->err_msg_.length();
    int32_t err_msg_len = ::htonl(message->err_msg_len_);
    memcpy(tmp, &err_msg_len, sizeof(int32_t));
    tmp += sizeof(err_msg_len);
    
    if (!message->err_msg_.empty()) {
        memcpy(tmp, message->err_msg_.c_str(), message->err_msg_len_);
        tmp += message->err_msg_len_;
    }
    
    if (!message->pb_data_.empty()) {
        memcpy(tmp, message->pb_data_.c_str(), message->pb_data_.length());
        tmp += message->pb_data_.length();
    }
    
    
    int32_t check_sum = ::htonl(1);
    memcpy(tmp, &check_sum, sizeof(int32_t));
    tmp += sizeof(check_sum);
    
    *tmp = TinyPBProtocol::PB_END_FLAG;
    
    len = pk_len;
    
    return buf;
}
}