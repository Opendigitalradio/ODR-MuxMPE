#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class General : public oatpp::DTO
{
  DTO_INIT(General, DTO)

  DTO_FIELD(Boolean, syslog);
  DTO_FIELD(Boolean, tsduck_debug);
};

class Input : public oatpp::DTO
{
  DTO_INIT(Input, DTO)

  DTO_FIELD(String, name);
  DTO_FIELD(String, protocol);
  DTO_FIELD(String, source);
  DTO_FIELD(Int32, sourceport);
  DTO_FIELD(String, sourceip);
};

class UnicastInput : public oatpp::DTO
{
  DTO_INIT(UnicastInput, DTO)

  DTO_FIELD(Int32, listenport);
  DTO_FIELD(String, mcast_address);
  DTO_FIELD(Int32, mcast_port);
};


class UnicastToMcast : public oatpp::DTO
{
  DTO_INIT(UnicastToMcast, DTO)

  DTO_FIELD(String, tap_interface);
  DTO_FIELD(String, tap_ip);
  DTO_FIELD(String, tap_subnet_mask);
  DTO_FIELD(List<Object<UnicastInput>>, inputs);
};

class Output : public oatpp::DTO
{
  DTO_INIT(Output, DTO)

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

class ConfigDTO : public oatpp::DTO
{
  DTO_INIT(ConfigDTO, DTO)

  DTO_FIELD(Object<General>, general);
  DTO_FIELD(Object<UnicastToMcast>, unicasttomcast);
  DTO_FIELD(Fields<Object<Input>>, inputs);
  DTO_FIELD(Object<Output>, output);
};

class FeedbackDto : public oatpp::DTO
{
  DTO_INIT(FeedbackDto, DTO)

  DTO_FIELD(String, name);
  DTO_FIELD(String, description);
};

#include OATPP_CODEGEN_END(DTO)
