#pragma once

#include <stdlib.h>

#include <fstream>
#include <functional>
#include <map>
#include <string>

#include "strings.hpp"

namespace Common {
class Config
{
public:
  using dumpHandler = std::function<void( const std::string&, const std::string&, const std::string& )>;
  void Dump( const dumpHandler& deal )
  {
    for ( const auto& [configKey, config] : cfg_ ) {
      for ( const auto& [key, value] : config ) {
        deal( configKey, key, value );
      }
    }
  }

  bool Load( const std::string& fileName )
  {
    if ( fileName.empty() ) {
      return false;
    }

    std::ifstream ifs;
    std::string line;
    ifs.open( fileName );
    if ( !ifs.is_open() ) {
      return false;
    }

    while ( std::getline( ifs, line ) ) {
      std::string section;
      std::string key;
      std::string value;
      if ( !parseLine( line, section, key, value ) ) {
        continue;
      }
      setSectionKeyValue( section, key, value );
    }

    return true;
  }

  void GetStrValue( const std::string& section,
                    const std::string& key,
                    std::string& value,
                    const std::string& defaultValue )
  {
    if ( cfg_.find( section ) == cfg_.end() ) {
      value = defaultValue;
      return;
    }

    if ( cfg_[section].find( key ) == cfg_[section].end() ) {
      value = defaultValue;
      return;
    }

    value = cfg_[section][key];
  }

  void GetIntValue( const std::string& section, const std::string& key, int64_t& value, int64_t defaultValue )
  {
    if ( cfg_.find( section ) == cfg_.end() ) {
      value = defaultValue;
      return;
    }

    if ( cfg_[section].find( key ) == cfg_[section].end() ) {
      value = defaultValue;
      return;
    }

    value = std::atoll( cfg_[section][key].c_str() );
  }

private:
  void setSectionKeyValue( const std::string& section, const std::string& key, const std::string& value )
  {
    if ( key.empty() || value.empty() ) {
      return;
    }
    cfg_[section][key] = value;
  }

  bool parseLine( std::string& line, std::string& section, std::string& key, std::string& value )
  {
    static std::string curSection;
    // 去掉注释的内容
    std::string nodes[2] { "#", ";" };
    for ( int i = 0; i < 2; ++i ) {
      if ( std::string::size_type pos = line.find( nodes[i] ); pos != std::string::npos ) {
        line.erase( pos );
      }
    }

    Strings::trim( line );
    if ( line.empty() ) {
      return false;
    }

    if ( line[0] == '[' && line[line.size() - 1] == ']' ) {
      section = line.substr( 1, line.size() - 2 );
      Strings::trim( section );
      curSection = section;
      return false;
    }

    if ( curSection == "" ) {
      return false;
    }

    bool isKey = true;
    for ( size_t i = 0; i < line.size(); ++i ) {
      if ( line[i] == '=' ) {
        isKey = false;
        continue;
      }

      if ( isKey ) {
        key += line[i];
      } else {
        value += line[i];
      }
    }

    section = curSection;
    Strings::trim( key );
    Strings::trim( value );

    return true;
  }

  std::map<std::string, std::map<std::string, std::string>> cfg_;
};
}  // namespace Common