#pragma once

#include <assert.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace Common {
class SockOpt
{
public:
  static void GetBufSize( int sockFd, int& readBufSize, int& writeBufSize )
  {
    int value = 0;
    socklen_t optlen;
    optlen = sizeof( value );
    getsockopt( sockFd, SOL_SOCKET, SO_RCVBUF, &value, &optlen );
    readBufSize = value;
    getsockopt( sockFd, SOL_SOCKET, SO_SNDBUF, &value, &optlen );
    writeBufSize = value;
  }

  static void SetBufSize( int sockFd, int readBufSize, int writeBufSize )
  {
    setsockopt( sockFd, SOL_SOCKET, SO_RCVBUF, &readBufSize, sizeof( readBufSize ) );
    setsockopt( sockFd, SOL_SOCKET, SO_SNDBUF, &writeBufSize, sizeof( writeBufSize ) );
  }

  static void EnableKeepAlive( int sockFd, int idleTime, int interval, int cnt )
  {
    int val = 1;
    auto len = static_cast<socklen_t>(sizeof( val ));
    setsockopt( sockFd, SOL_SOCKET, SO_KEEPALIVE, (void*)&val, len );

    val = idleTime;
    setsockopt( sockFd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&val, len );

    val = interval;
    setsockopt( sockFd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&val, len );

    val = cnt;
    setsockopt( sockFd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&val, len );
  }

  static void DisableNagle( int sockFd )
  {
    int noDelay = 1;
    auto len = static_cast<socklen_t>(sizeof( noDelay ));
    setsockopt( sockFd, IPPROTO_TCP, TCP_NODELAY, &noDelay, len );
  }

  static int GetSocketError( int sockFd )
  {
    int err = 0;
    socklen_t errLen = sizeof( err );
    getsockopt( sockFd, SOL_SOCKET, SO_ERROR, &err, &errLen );
    return err;
  }
};
}  // namespace Common
