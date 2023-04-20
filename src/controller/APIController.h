#ifndef APIController_hpp
#define APIController_hpp

#include "../MuxMPEWorker.h"
#include "dto/DTOs.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/protocol/http/incoming/Request.hpp"
#include "oatpp/web/protocol/http/outgoing/Response.hpp"
#include <iostream>

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- Begin Codegen

/**
 * Api Controller.
 */
class APIController : public oatpp::web::server::api::ApiController
{
public:
  /**
   * Constructor with object mapper.
   * @param objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  APIController(const std::shared_ptr<MuxMPEWorker> &myWorker, OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : m_myWorker(myWorker), oatpp::web::server::api::ApiController(objectMapper)
  {
  }

  static std::shared_ptr<APIController> createShared(const std::shared_ptr<MuxMPEWorker> &myService)
  {
    return std::make_shared<APIController>(myService);
  }

private:
  std::shared_ptr<MuxMPEWorker> m_myWorker;

public:
  ENDPOINT("GET", "/status", getStatus)
  {

    auto feedbackDto = FeedbackDto::createShared();

    if (m_myWorker->status())
    {
      feedbackDto->description = "Running";
      feedbackDto->name = "STATUS";
      return createDtoResponse(Status::CODE_200, feedbackDto);
    }
    else
    {
      feedbackDto->description = "Not Running";
      feedbackDto->name = "STATUS";
      return createDtoResponse(Status::CODE_200, feedbackDto);
    }
  }


  ENDPOINT("PUT", "/start", createStartup)
  {

    etiLog.log(info, "Starting up");
    auto feedbackDto = FeedbackDto::createShared();

    if (m_myWorker->startup())
    {
      etiLog.log(info, "Start Succesful");
      feedbackDto->description = "Start Succesful";
      feedbackDto->name = "STARTUP";
      return createDtoResponse(Status::CODE_201, feedbackDto);
    }
    else
    {
      etiLog.log(info, "Start FAILED");
      feedbackDto->description = "Start FAILED";
      feedbackDto->name = "STARTUP";
      return createDtoResponse(Status::CODE_201, feedbackDto);
    }
  }

  ENDPOINT("PUT", "/stop", stop)
  {

    etiLog.log(info, "Stopping");
    auto feedbackDto = FeedbackDto::createShared();

    if (m_myWorker->stop())
    {
      etiLog.log(info, "Stop Succesful");
      feedbackDto->description = "Stop Succesful";
      feedbackDto->name = "STOP";
      return createDtoResponse(Status::CODE_201, feedbackDto);
    }
    else
    {
      etiLog.log(info, "Stop Failed");
      feedbackDto->description = "Stop FAILED";
      feedbackDto->name = "STOP";
      return createDtoResponse(Status::CODE_201, feedbackDto);
    }
  }

  ENDPOINT("POST", "/config", postconfig, BODY_DTO(oatpp::Object<ConfigDTO>, config))
  {
    if (!m_myWorker->status())
    {
      try
      {
        etiLog.log(info, "Processing New Config");
        auto feedbackDto = FeedbackDto::createShared();

        if (!config)
        {
          // invalid input, return 400 status
          feedbackDto->description = "Configuration Empty";
          feedbackDto->name = "CONFIG";

          return createDtoResponse(Status::CODE_400, feedbackDto);
        }

        oatpp::parser::json::mapping::ObjectMapper mapper;
        std::string json = mapper.writeToString(config);

        m_myWorker->configureFromJSON(json);

        etiLog.log(info, "Configuration Succesful");
        feedbackDto->description = "Configuration Succesfully Updated";
        feedbackDto->name = "CONFIG";

        // configuration succeeded, return 200 status with success message
        return createDtoResponse(Status::CODE_200, feedbackDto);
      }
      catch (const std::exception &e)
      {
        auto feedbackDto = FeedbackDto::createShared();
        feedbackDto->description = e.what();
        feedbackDto->name = "CONFIG";

        etiLog.log(info, "Configuration FAILED");

        return createDtoResponse(Status::CODE_400, feedbackDto);
      }
    }
    else
    {
      auto feedbackDto = FeedbackDto::createShared();
      feedbackDto->description = "MPE is currently running, please stop first";
      feedbackDto->name = "CONFIG";

      etiLog.log(info, "Configuration FAILED due to running MPE");

      return createDtoResponse(Status::CODE_400, feedbackDto);
    }
  }

  ENDPOINT("GET", "/config", getConfig)
  {
   auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

    std::string jsonConfigStr = m_myWorker->getConfig();
    auto jsonConfig = oatpp::String(jsonConfigStr.c_str());

    auto configDto = objectMapper->readFromString<oatpp::Object<ConfigDTO>>(jsonConfig);

    return createDtoResponse(Status::CODE_200, configDto);
  }

};

#include OATPP_CODEGEN_END(ApiController) //<-- End Codegen

#endif /* APIController_hpp */
