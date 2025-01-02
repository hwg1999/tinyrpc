#pragma once

#include <functional>

namespace Common {
class Defer
{
public:
  explicit Defer( std::function<void( void )> func ) : func_( std::move( func ) ) {}
  ~Defer() { func_(); }

private:
  std::function<void( void )> func_;
};

}  // namespace Common