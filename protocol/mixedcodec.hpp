#pragma once

#include "common/convert.hpp"
#include "common/statuscode.hpp"
#include "httpcodec.hpp"
#include "mysvrcodec.hpp"
#include "protocol/codec.hpp"
#include "protocol/httpmessage.hpp"
#include "protocol/mysvrmessage.hpp"
#include <memory>
#include <string>

namespace Protocol {
class MixedCodec : public Codec
{
public:
  CodecType Type() override
  {
    if ( nullptr == codec_ ) {
      return UNKNOWN;
    }
    return codec_->Type();
  }

  uint8_t* Data()
  {
    // 无法确定具体协议之前，只读取一个字节
    if ( nullptr == codec_ ) {
      return &first_byte_;
    }
    return codec_->Data();
  }

  size_t Len()
  {
    // 无法确定具体协议之前，只读取一个字节
    if ( nullptr == codec_ ) {
      return 1;
    }
    return codec_->Len();
  }

  void* GetMessage() override
  {
    if ( nullptr == codec_ ) {
      return nullptr;
    }
    return codec_->GetMessage();
  }

  bool Encode( void* msg, Packet& pkt ) override
  {
    if ( nullptr == codec_ ) {
      return false;
    }
    return codec_->Encode( msg, pkt );
  }

  bool Decode( size_t len ) override
  {
    assert( len >= 1 );
    createCodec();
    assert( codec_ != nullptr );
    return codec_->Decode( len );
  }

  static void Http2MySvr( HttpMessage& httpMessage, MySvrMessage& mySvrMessage )
  {
    mySvrMessage.context_.set_service_name( httpMessage.GetHeader( "service_name" ) );
    mySvrMessage.context_.set_rpc_name( httpMessage.GetHeader( "rpc_name" ) );
    mySvrMessage.BodyEnableJson(); // body的格式设置为json
    size_t bodyLen = httpMessage.body_.size();
    mySvrMessage.body_.Alloc( bodyLen );
    memmove( mySvrMessage.body_.Data(), httpMessage.body_.data(), bodyLen );
    mySvrMessage.body_.UpdateUseLen( bodyLen );
  }

  static void MySvr2Http( MySvrMessage& mySvrMessage, HttpMessage& httpMessage )
  {
    httpMessage.SetStatusCode( OK );
    httpMessage.SetHeader( "log_id", mySvrMessage.context_.log_id() );
    httpMessage.SetHeader( "status_code", std::to_string( mySvrMessage.context_.status_code() ) );
    if ( 0 == mySvrMessage.context_.status_code() ) {
      assert( mySvrMessage.BodyIsJson() ); // body此时必须是json str
      size_t len = mySvrMessage.body_.UseLen();
      httpMessage.SetBody( std::string( reinterpret_cast<char*>( mySvrMessage.body_.DataRaw() ), len ) );
    } else {
      httpMessage.SetBody( R"({"message":")" + mySvrMessage.Message() + R"("})" );
    }
  }

  static bool PbParseFromMySvr( google::protobuf::Message& pb, MySvrMessage& mySvr )
  {
    std::string str( reinterpret_cast<char*>( mySvr.body_.DataRaw() ), mySvr.body_.UseLen() );
    if ( mySvr.BodyIsJson() ) { // json格式
      return Common::Convert::JsonStr2Pb( str, pb );
    }
    return pb.ParseFromString( str );
  }

  static void PbSerializeToMySvr( google::protobuf::Message& pb, MySvrMessage& mySvr, int statusCode )
  {
    std::string str;
    bool result = true;
    if ( mySvr.BodyIsJson() ) { // json格式
      result = Common::Convert::Pb2JsonStr( pb, str );
    } else {
      result = pb.SerializeToString( &str );
    }

    if ( !result ) {
      mySvr.context_.set_status_code( SERIALIZE_FAILED );
      return;
    }

    mySvr.body_.Alloc( str.size() );
    memmove( mySvr.body_.Data(), str.data(), str.size() );
    mySvr.body_.UpdateUseLen( str.size() );
    mySvr.context_.set_status_code( statusCode );
  }

  static void JsonStrSerializeToMySvr( const std::string& serviceName,
                                       const std::string& rpcName,
                                       std::string& jsonStr,
                                       MySvrMessage& mySvr )
  {
    mySvr.BodyEnableJson();
    mySvr.context_.set_service_name( serviceName );
    mySvr.context_.set_rpc_name( rpcName );
    mySvr.body_.Alloc( jsonStr.size() );
    memmove( mySvr.body_.Data(), jsonStr.data(), jsonStr.size() );
    mySvr.body_.UpdateUseLen( jsonStr.size() );
  }

private:
  void createCodec()
  {
    if ( codec_ != nullptr ) {
      return;
    }

    if ( PROTO_MAGIC_AND_VERSION == first_byte_ ) {
      codec_ = std::make_unique<MySvrCodec>();
    } else {
      codec_ = std::make_unique<HttpCodec>();
    }
    memmove( codec_->Data(), &first_byte_, 1 ); // 拷贝1个字节的内容
    first_byte_ = 0;
  }

  std::unique_ptr<Codec> codec_ { nullptr };
  uint8_t first_byte_ { 0 };
};

} // namespace Protocol