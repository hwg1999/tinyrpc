#pragma once

#include "packet.hpp"

namespace Protocol {
// 编解码解析协议的类型
enum CodecType
{
  UNKNOWN = 0,
  HTTP = 1,
  MY_SVR = 2,
  RESP = 3,
};

// 协议编解码基类
class Codec
{
public:
  virtual ~Codec() {}
  uint8_t* Data() { return pkt_.Data(); }
  size_t Len() { return pkt_.Len(); }
  virtual void* GetMessage() = 0;
  virtual bool Encode( void* msg, Packet& pkt ) = 0;
  virtual bool Decode( size_t len ) = 0;
  virtual CodecType Type() = 0;

protected:
  Packet pkt_;
};
} // namespace Protocol