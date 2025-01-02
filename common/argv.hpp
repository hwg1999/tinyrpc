#pragma once

#include <string>
#include <unordered_map>

namespace Common {
class Argv
{
public:
  Argv& Set( const std::string& name, void* arg )
  {
    argv_[name] = arg;
    return *this;
  }

  template<typename T>
  T& Arg( const std::string& name )
  {
    if ( argv_.find( name ) == argv_.end() ) {
      return {};
    }

    return *static_cast<T*>( argv_[name] );
  }

private:
  std::unordered_map<std::string, void*> argv_;
};
} // namespace Common