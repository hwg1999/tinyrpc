#include "cmdline.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace Common::CmdLine {

// 前置声明
class Opt;
static Usage usage_ = nullptr;
static std::map<std::string, Opt> opts_;

enum OptType
{
  INT64 = 1,
  BOOL = 2,
  STRING = 3
};

class Opt
{
public:
  Opt() = default;

  template<typename T>
  Opt( const std::string& name, bool required, T value, T defaultValue ) : Opt( name, std::move(value), required )
  {
    value_ = defaultValue;
  }

  template<typename T>
  void SetValue( T& value )
  {
    if constexpr ( std::is_same_v<T, std::string> ) {
      value_is_set_ = true;
      if ( std::holds_alternative<std::string>( value_ ) ) {
        value_ = value;
      } else {
        value_ = std::atoll( value.c_str() );
      }
    } else if constexpr ( std::is_same_v<T, bool> ) {
      value_is_set_ = true;
      value_ = value;
    }
  }

  bool CheckRequired() const
  {
    if ( !required_ ) {
      return true;
    }

    if ( required_ && value_is_set_ ) {
      return true;
    }

    printf( "required option %s not set argument\n", name_.c_str() );
    return false;
  }

  bool IsBoolOpt() const { return std::holds_alternative<bool>( value_ ); }

  void SetBoolValue( bool value )
  {
    value_is_set_ = true;
    value_ = value;
  }
  void SetValue( std::string value )
  {
    if ( std::holds_alternative<std::string>( value_ ) ) {
      value_ = value;
    } else if ( std::holds_alternative<int64_t>( value_ ) ) {
      value_ = std::atoll( value.c_str() );
    }

    value_is_set_ = true;
  }

private:
  template<typename T>
  Opt( std::string name, T value, bool required ) : value_ { value }, name_ {std::move( name )}, required_ { required }
  {
    if ( required_ ) {
      value_is_set_ = false;
    }
  }

  std::string name_;
  mutable std::variant<int64_t, bool, std::string> value_;
  bool value_is_set_ { true };
  bool required_ { false };
};

static bool isValidName( const std::string& name )
{
  if ( name.empty() ) {
    return false;
  }

  if ( name[0] == '-' ) {
    printf( "option %s begins with -\n", name.c_str() );
    return false;
  }

  if ( name.find( '=' ) != std::string::npos ) {
    printf( "options %s contains =\n", name.c_str() );
    return false;
  }

  return true;
}

static int ParseOpt( int argc, char* argv[], int& parseIndex )
{
  char* optStr = argv[parseIndex];
  int optLen = strlen( optStr );
  // 选项的长度必须>=2
  if ( optLen <= 1 ) {
    printf( "option's len must greater than or equal to 2\n" );
    return -1;
  }

  // 选项必须以'-'开头
  if ( optStr[0] != '-' ) {
    printf( "option must begins with '-', %s is invalid option\n", optStr );
    return -1;
  }

  // 过滤第一个'-'
  optStr++;
  optLen--;
  if ( *optStr == '-' ) {
    optStr++; // 过滤第二个'-'
    optLen--;
  }

  // 过滤完有效的'-'之后还要再check一下后面的内容和长度
  if ( optLen == 0 || *optStr == '-' || *optStr == '=' ) {
    printf( "bad opt syntax:%s\n", argv[parseIndex] );
    return -1;
  }

  // 执行到这里说明是一个选项，接下来判断这个选项是否有参数
  bool hasArgument = false;
  std::string argument;
  for ( int i = 1; i < optLen; ++i ) {
    if ( optStr[i] == '=' ) {
      hasArgument = true;
      argument = std::string( optStr + i + 1 ); // 取等号之后的内容赋值为argument
      optStr[i] = 0;                            // 这样opt指向的字符串就是'='之前的内容。
      break;
    }
  }

  std::string optName { optStr };
  // 有help选项，则直接调用usage_函数，并退出
  if ( optName == "help" || optName == "h" ) {
    if ( usage_ != nullptr ) {
      usage_();
    }
    exit( 0 );
  }

  // 选项不存在
  if ( opts_.find( optName ) == opts_.end() ) {
    printf( "option provided but not defined: -%s\n", optName.c_str() );
    return -1;
  }

  auto& opt = opts_[optName];
  // 不需要参数的bool类型选项
  if ( opt.IsBoolOpt() ) {
    opt.SetBoolValue( true );
    parseIndex++; // parseIndex跳到下一个选项
  } else {        // 需要参数的选项，参数可能在下一个命令行参数中
    if ( hasArgument ) {
      parseIndex++;
    } else {
      if ( parseIndex + 1 < argc ) { // 选项的值在下一个命令行参数
        hasArgument = true;
        argument = std::string( argv[parseIndex + 1] );
        parseIndex += 2; // parseIndex跳到下一个选项
      }
    }

    if ( !hasArgument ) {
      printf( "option needs an argument: -%s\n", optName.c_str() );
      return -1;
    }
    opt.SetValue( argument );
  }

  return 0;
}

static bool CheckRequired()
{
  for ( const auto& [_, opt] : opts_ ) {
    if ( !opt.CheckRequired() ) {
      return false;
    }
  }

  return true;
}

static void setOptCheck( const std::string& name )
{
  if ( opts_.find( name ) != opts_.end() ) {
    printf( "%s opt already set\n", name.c_str() );
    exit( -1 );
  }

  if ( !isValidName( name ) ) {
    printf( "%s is invalid name\n", name.c_str() );
    exit( -2 );
  }
}

void BoolOpt( bool value, const std::string& name )
{
  setOptCheck( name );
  bool defaultValue = false;
  opts_[name] = Opt( name, false, value, defaultValue );
}

void Int64Opt( int64_t value, const std::string& name, int64_t defaultValue )
{
  setOptCheck( name );
  opts_[name] = Opt( name, false, value, defaultValue );
}

void StrOpt( const std::string& value, const std::string& name, const std::string& defaultValue )
{
  setOptCheck( name );
  opts_[name] = Opt( name, false, value, defaultValue );
}

void Int64OptRequired( int64_t value, const std::string& name )
{
  setOptCheck( name );
  int64_t defaultValue = 0;
  opts_[name] = Opt( name, true, value, defaultValue );
}

void StrOptRequired( const std::string& value, const std::string& name )
{
  setOptCheck( name );
  std::string defaultValue;
  opts_[name] = Opt( name, true, value, defaultValue );
}

void SetUsage( Usage usage )
{
  usage_ = usage;
}

void Parse( int argc, char** argv )
{
  if ( nullptr == usage_ ) {
    printf( "usage function not set\n" );
    exit( -1 );
  }
  // 这里跳过命令名不解析，所以parseIndex从1开始
  int parseIndex = 1;
  while ( parseIndex < argc ) {
    if ( ParseOpt( argc, argv, parseIndex ) != 0 ) {
      exit( -2 );
    }
  }

  // 校验必设选项，非必设的则设置默认值
  if ( !CheckRequired() ) {
    usage_();
    exit( -1 );
  }
}

} // namespace Common::CmdLine
