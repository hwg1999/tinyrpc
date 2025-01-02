#pragma once

#include "utils.hpp"
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include <unistd.h>

namespace Common {
class RobustIo
{
public:
  explicit RobustIo( int fd ) : fd_ { fd } {}

  ssize_t Write( uint8_t* data, size_t len )
  {
    ssize_t total = len;
    while ( total > 0 ) {
      ssize_t ret = write( fd_, data, total );
      if ( ret <= 0 ) {
        if ( 0 == ret || RestartAgain( errno ) ) {
          continue;
        }
        return -1;
      }

      total -= ret;
      data += ret;
    }

    return len;
  }

  ssize_t Read( uint8_t* data, size_t len )
  {
    ssize_t total = len;
    while ( total > 0 ) {
      ssize_t ret = read( fd_, data, total );
      if ( 0 == ret ) {
        break;
      }

      if ( ret < 0 ) {
        if ( RestartAgain( errno ) ) {
          continue;
        }
        return -1;
      }

      total -= ret;
      data += ret;
    }

    return len - total;
  }

  void SetNotBlock()
  {
    Utils::SetNotBlock( fd_ );
    is_block_ = false;
  }

  void SetTimeOut( int64_t timeOutSec, int64_t timeoutUSec ) const
  {
    // 非阻塞的不用设置sock的读写超时时间，设置了也无效果
    if ( !is_block_ ) {
      return;
    }

    struct timeval tv { .tv_sec = timeOutSec, .tv_usec = timeoutUSec };
    setsockopt( fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( tv ) );
    setsockopt( fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof( tv ) );
  }

  bool RestartAgain( int err ) const
  {
    // 被信号中断都可以重启读写
    if ( EINTR == err ) {
      return true;
    }

    // 阻塞io的情况下，其他情况都不可以重启读写
    if ( is_block_ ) {

      return false;
    }

    if ( EAGAIN == err || EWOULDBLOCK == err ) {

      return true;
    }

    return false;
  }

private:
  int fd_ { -1 };
  // fd_默认是阻塞的
  bool is_block_ { true };
};
}  // namespace Common