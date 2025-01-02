#pragma once

#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>

namespace Common {
class Utils
{
public:
  static std::string GetSelfName()
  {
    char buf[1024] { 0 };
    ssize_t ret = readlink( "/proc/self/exe", buf, 1023 );
    assert( ret > 0 );
    buf[ret] = '\0';
    if ( char* begin = strrchr( buf, '/' ); begin != nullptr ) {
      return begin;
    }

    return buf;
  }

  static std::string GetIpStr( const std::string& ethName )
  {
    struct ifaddrs* ifList = nullptr;
    if ( getifaddrs( &ifList ) < 0 ) {
      return "";
    }

    char ip[20] { 0 };
    std::string ipStr = "127000000001";
    for ( struct ifaddrs* ifa = ifList; ifa != nullptr; ifa = ifa->ifa_next ) {
      if ( ifa->ifa_addr->sa_family != AF_INET ) {
        continue;
      }

      if ( strcmp( ifa->ifa_name, ethName.c_str() ) == 0 ) {
        auto* addr = reinterpret_cast<struct sockaddr_in*>( ifa->ifa_addr );
        auto* sin_addr = reinterpret_cast<uint8_t*>( &addr->sin_addr );
        snprintf( ip, 20, "%03d%03d%03d%03d", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3] );
        ipStr = ip;
        break;
      }
    }

    freeifaddrs( ifList );

    return ipStr;
  }

  static uint32_t GetAddr( const std::string& ethName )
  {

    struct ifaddrs* ifList = nullptr;
    uint32_t ethAddr = htonl( INADDR_LOOPBACK );
    if ( ethName == "any" ) {
      return htonl( INADDR_ANY );
    }

    if ( ethName == "lo" || getifaddrs( &ifList ) < 0 ) {
      return ethAddr;
    }

    for ( struct ifaddrs* ifa = ifList; ifa != nullptr; ifa = ifa->ifa_next ) {
      if ( ifa->ifa_addr->sa_family != AF_INET ) {
        continue;
      }
      if ( strcmp( ifa->ifa_name, ethName.c_str() ) == 0 ) {
        auto* addr = reinterpret_cast<struct sockaddr_in*>( ifa->ifa_addr );
        ethAddr = addr->sin_addr.s_addr;
        break;
      }
    }

    freeifaddrs( ifList );

    return ethAddr;
  }

  static void CoreDumpEnable()
  {
    struct rlimit rlim_new { 0, 0 };
    rlim_new.rlim_cur = RLIM_INFINITY;
    rlim_new.rlim_max = RLIM_INFINITY;
    setrlimit( RLIMIT_CORE, &rlim_new );
  }

  static void SetNotBlock( int fd )
  {
    int oldOpt = fcntl( fd, F_GETFL );
    assert( oldOpt != -1 );
    assert( fcntl( fd, F_SETFL, oldOpt | O_NONBLOCK ) != -1 );
  }

  static int GetNProcs() { return get_nprocs(); }
};

} // namespace Common