#pragma once

#include <string>

namespace Common::CmdLine {
using Usage = void ( * )();

// 非必设的bool类型的选项，默认值都是false，bool选项是命令行的功能开关
void BoolOpt( bool value, const std::string& name );
// 非必设的int64_t类型的选项，可以指定默认值
void Int64Opt( int64_t value, const std::string& name, int64_t defaultValue );
// 非必设的string类型的选项，可以指定默认值
void StrOpt( const std::string& value, const std::string& name, const std::string& defaultValue );
// 必设的int64_t类型的选项
void Int64OptRequired( int64_t value, const std::string& name );
// 必设的string类型的选项
void StrOptRequired( const std::string& value, const std::string& name );
void SetUsage( Usage usage );
void Parse( int argc, char* argv[] );

} // namespace Common::CmdLine
