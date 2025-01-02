#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <iterator>

#include <algorithm>
#include <string>
#include <vector>

namespace Common {
class Strings
{
public:
  static void ltrim( std::string& str )
  {
    if ( str.empty() ) {
      return;
    }
    str.erase( 0, str.find_first_not_of( ' ' ) );
  }

  static void rtrim( std::string& str )
  {
    if ( str.empty() ) {
      return;
    }
    str.erase( str.find_last_not_of( ' ' ) + 1 );
  }

  static void trim( std::string& str )
  {
    ltrim( str );
    rtrim( str );
  }

  static void Split( std::string& str, const std::string& sep, std::vector<std::string>& result )
  {
    if ( str.empty() ) {
      return;
    }

    std::string::size_type prePos = 0;
    std::string::size_type curPos = str.find( sep );
    while ( std::string::npos != curPos ) {
      std::string subStr = str.substr( prePos, curPos - prePos );
      // 非空串才插入
      if ( subStr != "" ) {
        result.push_back( subStr );
      }
      prePos = curPos + sep.size();
      curPos = str.find( sep, prePos );
    }

    if ( prePos != str.length() ) {
      result.push_back( str.substr( prePos ) );
    }
  }

  static std::string Join( std::vector<std::string>& strs, const std::string& sep )
  {
    std::string result;
    for ( size_t i = 0; i < strs.size(); ++i ) {
      result += strs[i];
      if ( i != strs.size() - 1 ) {
        result += sep;
      }
    }

    return result;
  }

  static std::string StrFormat( const char* format, ... )
  {
    std::vector<char> buf( 1024 );
    va_list plist;
    va_start( plist, format );
    int ret = vsnprintf( buf.data(), buf.size(), format, plist );
    va_end( plist );

    assert( ret > 0 );
    if ( ret >= 1024 ) {
      buf.resize( ret + 1 );
      va_start( plist, format );
      ret = vsnprintf( buf.data(), ret + 1, format, plist );
      va_end( plist );
    }

    return buf.data();
  }

  static void ToLower( std::string& str )
  {
    std::transform( std::begin( str ), std::end( str ), std::begin( str ), ::tolower );
  }
};

}  // namespace Common