#pragma once

#include "log.hpp"
#include "robustio.hpp"
#include <fcntl.h>
#include <unistd.h>

namespace Common {
class ServiceLock
{
  static bool lock( const std::string& pidFile )
  {
    int fd = open( pidFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
    if ( fd < 0 ) {
      ERROR( "open %s failed, error:%s", pidFile.c_str(), strerror( errno ) );
      return false;
    }

    // 返回0表示未加锁或者被当前进程加锁；返回-1表示被其他进程加锁
    int ret = lockf( fd, F_TEST, 0 );
    if ( ret < 0 ) {
      ERROR( "lock %s failed, error:%s", pidFile.c_str(), strerror( errno ) );
      return false;
    }

    // 尝试给文件加锁，加锁失败直接返回错误码，而不是一直阻塞
    ret = lockf( fd, F_TLOCK, 0 );
    if ( ret < 0 ) {
      ERROR( "lock %s failed, error:%s", pidFile.c_str(), strerror( errno ) );
      return false;
    }

    // 清空文件
    ftruncate( fd, 0 );
    // 移动到文件开头
    lseek( fd, 0, SEEK_SET );

    char buf[1024] = { 0 };
    sprintf( buf, "%d", getpid() );
    RobustIo robustIo( fd );
    // 写入进程的pid
    robustIo.Write( reinterpret_cast<uint8_t*>(buf), strlen( buf ) );
    INFO( "lock pidFile[%s] success. pid[%s]", pidFile.c_str(), buf );

    return true;
  }
};
}