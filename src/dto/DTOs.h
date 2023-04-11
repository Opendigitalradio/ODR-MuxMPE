#ifndef DTOs_h
#define DTOs_h

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)


class GeneralDTO : public oatpp::DTO {

  DTO_INIT(GeneralDTO, DTO)

  DTO_FIELD(Boolean, syslog);
  DTO_FIELD(Boolean, tsduck_debug);

};

class InputDTO : public oatpp::DTO {

  DTO_INIT(InputDTO, DTO)

  DTO_FIELD(String, name);
  DTO_FIELD(String, protocol);
  DTO_FIELD(String, source);
  DTO_FIELD(Int32, sourceport);
  DTO_FIELD(String, sourceip);

};

class OutputDTO : public oatpp::DTO {

  DTO_INIT(OutputDTO, DTO)

  DTO_FIELD(String, protocol);
  DTO_FIELD(Int32, bitrate);
  DTO_FIELD(Int32, ts_id);
  DTO_FIELD(Int32, service_id);
  DTO_FIELD(String, service_name);
  DTO_FIELD(String, service_provider_name);
  DTO_FIELD(Int32, payload_pid);
  DTO_FIELD(Int32, pmt_pid);
  DTO_FIELD(String, output);
  DTO_FIELD(String, output_host);
  DTO_FIELD(Int32, output_port);
  DTO_FIELD(String, output_source_address);
  DTO_FIELD(String, output_srt_passphrase);
  DTO_FIELD(Int32, output_ttl);

};

class ConfigDTO : public oatpp::DTO {

  DTO_INIT(ConfigDTO, DTO)

  DTO_FIELD(String, _comment);
  DTO_FIELD(Object<GeneralDTO>, general);
  DTO_FIELD(Fields<Object<InputDTO>>, input, "inputs");
  DTO_FIELD(Object<OutputDTO>, output);

};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class MyDto : public oatpp::DTO {
  
  DTO_INIT(MyDto, DTO)
  DTO_FIELD(Int32, statusCode);
  DTO_FIELD(String, message);
  
};

class ErrorDto : public oatpp::DTO {
  DTO_INIT(ErrorDto, DTO)
  DTO_FIELD(String, message);
};

class FeedbackDto : public oatpp::DTO {

  DTO_INIT(FeedbackDto, DTO)

  DTO_FIELD(String, name);
  DTO_FIELD(String, description);

};

#include OATPP_CODEGEN_END(DTO)

#endif /* DTOs_hpp */