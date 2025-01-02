#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <memory>
#include <sstream>

namespace Common {
class Convert
{
public:
  static bool Pb2JsonStr( const google::protobuf::Message& message, std::string& jsonStr, bool addWhitespace = false )
  {
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = addWhitespace;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    options.always_print_enums_as_ints = true;
    return google::protobuf::util::MessageToJsonString( message, &jsonStr, options ).ok();
  }

  static bool JsonStr2Pb( const std::string& jsonStr, google::protobuf::Message& message )
  {
    return google::protobuf::util::JsonStringToMessage( jsonStr, &message ).ok();
  }

  static bool Pb2Json( const google::protobuf::Message& message, Json::Value& value )
  {
    std::string jsonStr;
    if ( !Pb2JsonStr( message, jsonStr ) ) {
      return false;
    }

    Json::CharReaderBuilder readerBuilder;
    std::istringstream iss { jsonStr };
    std::string errs;
    return Json::parseFromStream( readerBuilder, iss, &value, &errs );
  }

  static bool Json2Pb( Json::Value& value, google::protobuf::Message& message )
  {
    Json::StreamWriterBuilder writerBuilder;
    std::unique_ptr<Json::StreamWriter> writer { writerBuilder.newStreamWriter() };
    std::ostringstream oss;
    writer->write( value, &oss );
    std::string jsonStr = oss.str();
    return JsonStr2Pb( jsonStr, message );
  }
};
}  // namespace Common