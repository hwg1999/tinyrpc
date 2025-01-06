#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "codec.hpp"

namespace Protocol {
// 只支持4种应答类型
enum RedisReplyType
{
  SIMPLE_STRINGS = 1, // 简单字符串，响应的首字节是 "+"，demo: "+OK\r\n"
  ERRORS = 2,         // 错误，响应的首字节是 "-"，demo："-Error message\r\n"
  INTEGERS = 3,       // 整型， 响应的首字节是 ":"，demo：":1000\r\n"
  BULK_STRINGS = 4,   // 批量字符串，相应的首字节是 "$"，demo："$2\r\nok\r\n"，"$0\r\n\r\n"，"$-1\r\n"
};

/* 只支持5种命令
  SET  // 设置key，value
  GET  // 获取key，对应的value值
  DEL  // 删除key
  INCR // 数值递增1
  AUTH // 认证
 */
struct RedisCommand
{
  void makeGetCmd( std::string key )
  {
    params_.emplace_back( "GET" );
    params_.emplace_back( key );
  }

  void makeDelCmd( std::string key )
  {
    params_.emplace_back( "DEL" );
    params_.emplace_back( key );
  }

  void makeAuthCmd( std::string passwd )
  {
    params_.emplace_back( "AUTH" );
    params_.emplace_back( passwd );
  }

  void makeSetCmd( std::string key, std::string value, int64_t expireTime = 0 )
  {
    params_.emplace_back( "SET" );
    params_.emplace_back( key );
    params_.emplace_back( value );
    if ( expireTime > 0 ) {
      params_.emplace_back( "EX" );
      params_.emplace_back( std::to_string( expireTime ) );
    }
  }

  void makeIncrCmd( const std::string& key )
  {
    params_.emplace_back( "INCR" );
    params_.push_back( key );
  }

  void GetOut( std::string& str )
  {
    size_t len = params_.size();
    std::stringstream out;
    out << "*" << len << "\r\n";
    for ( const auto& param : params_ ) {
      out << "$" << param.size() << "\r\n";
      out << param << "\r\n";
    }
    str = out.str();
  }

  std::vector<std::string> params_;
};

struct RedisReply
{
  bool IsOk() const { return value_ == "OK"; }
  bool IsError() const { return type_ == ERRORS; }
  bool IsNull() const { return is_null_; }
  std::string Value() const { return value_; }
  int64_t IntValue() const
  {
    assert( type_ == INTEGERS );
    return std::stol( value_ );
  }

  RedisReplyType type_;
  std::string value_;
  bool is_null_ { false };
};

} // namespace Protocol