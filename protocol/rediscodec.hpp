#pragma once

#include "codec.hpp"
#include "redismessage.hpp"
#include <cstdlib>
#include <memory>

namespace Protocol {
// 解析状态
enum ReplyDecodeStatus
{
  FIRST_CHAR = 1,   // 第一个字符
  SIMPLE_VALUE = 2, // reply单行值
  BULK_VALUE = 3,   // 批量字符串值
  END = 4,          // 完成了消息解析
};

// 协议编解码
class RedisCodec : public Codec
{
public:
  RedisCodec() { pkt_.Alloc( 100 ); }
  ~RedisCodec() = default;

  CodecType Type() { return RESP; }
  void* GetMessage()
  {
    if ( nullptr == message_ ) {
      return nullptr;
    }
    if ( decode_status_ != END ) {
      return nullptr;
    }

    decode_status_ = FIRST_CHAR; // 消息被取出之后解析状态设置为FIRST_CHAR
    return message_.release();
  }

private:
  bool decodeFirstChar( uint8_t** data, uint32_t& needDecodeLen, uint32_t& decodeLen, bool& decodeBreak )
  {
    if ( needDecodeLen < 1 ) {
      decodeBreak = true;
      return true;
    }

    char* curData = (char*)*data;
    if ( *curData == '+' ) {
      message_->type_ = SIMPLE_STRINGS;
    } else if ( *curData == '-' ) {
      message_->type_ = ERRORS;
    } else if ( *curData == ':' ) {
      message_->type_ = INTEGERS;
    } else if ( *curData == '$' ) {
      message_->type_ = BULK_STRINGS;
    } else {
      assert( 0 );
    }

    // 更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= 1;
    decodeLen += 1;
    ( *data ) += 1;
    if ( BULK_STRINGS == message_->type_ ) {
      decode_status_ = BULK_VALUE;
    } else {
      decode_status_ = SIMPLE_VALUE;
    }
    return true;
  }

  bool decodeSimpleValue( uint8_t** data, uint32_t& needDecodeLen, uint32_t& decodeLen, bool& decodeBreak )
  {
    char* curData = (char*)( *data );
    bool getValue = false;
    uint32_t currentDecodeLen = 0;
    for ( uint32_t i = 0; i < needDecodeLen - 1; ++i ) {
      if ( curData[i] == '\r' && curData[i + 1] == '\n' ) {
        curData[i] = 0;
        currentDecodeLen = i + 2;
        message_->value_ = std::string( curData );
        getValue = true;
      }
    }

    decodeBreak = true;
    if ( !getValue ) {
      pkt_.ReAlloc( pkt_.UseLen() * 2 ); // 无法完成value的解析，则尝试扩大下次读取的数据量
      return true;
    }

    // 更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= currentDecodeLen;
    decodeLen += currentDecodeLen;
    ( *data ) += currentDecodeLen;
    decode_status_ = END;
    assert( needDecodeLen == 0 );

    return true;
  }

  bool decodeBulkValue( uint8_t** data, uint32_t& needDecodeLen, uint32_t& decodeLen, bool& decodeBreak )
  {
    char* curData = (char*)( *data );
    int32_t bulkLen = 0;
    bool getBulkLen = false;
    bool getBulkValue = false;
    uint32_t currentDecodeLen = 0;
    for ( uint32_t i = 0; i < needDecodeLen - 1; ++i ) {
      if ( curData[i] != '\r' || curData[i + 1] != '\n' ) {
        continue;
      }

      // 还没解析到长度
      if ( !getBulkLen ) {
        getBulkLen = true;
        curData[i] = 0;
        bulkLen = std::atoi( curData );
        // bulk string 最大长度为512M
        if ( bulkLen > 512 * 1024 * 1024 ) {
          return false;
        }
        curData[i] = '\r';     // 取完长度之后需要设置回去
        if ( bulkLen == -1 ) { // null值
          message_->is_null_ = true;
          currentDecodeLen = i + 2;
          getBulkValue = true;
          break;
        }
      } else { // 解析到具体值（空值，或者非空值）
        getBulkValue = true;
        currentDecodeLen = i + 2;
        if ( 0 == bulkLen ) { // 空值
          message_->value_ = "";
          break;
        }
        curData[i] = 0;                                              // 先设置字符串结束标志
        message_->value_ = std::string( curData + ( i - bulkLen ) ); // 取字符串
        curData[i] = '\r';                                           // 取完字符串之后需要设置回去
        break;
      }
    }

    decodeBreak = true; // 不管是否能完成解析都跳出循环
    if ( !getBulkValue ) {
      pkt_.ReAlloc( pkt_.UseLen() * 2 ); // 无法完成value的解析，则尝试扩大下次读取的数据量
      return true;
    }

    // 更新剩余待解析数据长度，已经解析的长度，缓冲区指针的位置，当前解析的状态。
    needDecodeLen -= currentDecodeLen;
    decodeLen += currentDecodeLen;
    ( *data ) += currentDecodeLen;
    decode_status_ = END;
    assert( needDecodeLen == 0 );

    return true;
  }

  ReplyDecodeStatus decode_status_ { FIRST_CHAR };
  std::unique_ptr<RedisReply> message_ { nullptr };
};

} // namespace Protocol