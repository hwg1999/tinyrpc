#pragma once

namespace Common {

template<typename T>
class Singleton
{
public:
  static T& Instance()
  {
    static T instance;
    return instance;
  }
};

}  // namespace Common