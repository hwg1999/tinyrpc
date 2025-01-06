#pragma once

#include "base.pb.h"
#include <cstddef>
#include <cstdlib>

namespace Protocol {
class Packet
{
public:
  void Alloc( size_t len )
  {
    data_.resize( len );
    len_ = len;
    use_len_ = 0;
    parse_len_ = 0;
  }

  void ReAlloc( size_t len )
  {
    if ( len < len_ ) {
      return;
    }

    data_.resize( len );
    len_ = len;
  }

  void CopyFrom( const Packet& pkt )
  {
    data_ = pkt.data_;
    len_ = pkt.len_;
    use_len_ = pkt.use_len_;
    parse_len_ = pkt.parse_len_;
  }

  uint8_t* Data() { return data_.data() + use_len_; }           // 缓冲区可以写入的开始地址
  uint8_t* DataRaw() { return data_.data(); }                   // 原始缓冲区的开始地址
  uint8_t* DataParse() { return data_.data() + parse_len_; }    // 需要解析的开始地址
  size_t NeedParseLen() const { return use_len_ - parse_len_; } // 还需要解析的长度
  size_t Len() const { return len_ - use_len_; }                // 缓存区中还可以写入的数据长度
  size_t UseLen() const { return use_len_; }                    // 缓冲区已经使用的容量
  void UpdateUseLen( size_t add_len ) { use_len_ += add_len; }
  void UpdateParseLen( size_t add_len ) { parse_len_ += add_len; }

private:
  std::vector<uint8_t> data_; // 缓冲区
  size_t len_ { 0 };          // 缓冲区的长度
  size_t use_len_ { 0 };      // 缓冲区使用长度
  size_t parse_len_ { 0 };    // 完成解析的长度
};
} // namespace Protocol::Packet