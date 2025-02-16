#ifndef _SERVER_HPP_
#define _SERVER_HPP_
#include <ESPAsyncWebServer.h>

#include <Controller.hpp>

#include "WsResponse.hpp"
class MyServer {
 private:
  /* data */
  xTaskHandle serverTaskHandle = 0;
  xTaskHandle wsTaskHandle = 0;
  xTaskHandle broadcastTaskHandle = 0;
  std::unique_ptr<AsyncWebServer> server;
  std::unique_ptr<AsyncWebSocket> ws;
  std::shared_ptr<Controller> controller;

  const ArRequestHandlerFunction getParametersHandler =
      [this](AsyncWebServerRequest* request) {
        auto gain = controller->getGain();
        request->send(200, "application/json", gain.toJson());
      };

  void returnInvalidRequest(AsyncWebServerRequest* request) {
    request->send(400, "application/json",
                  "{\"message\",\"invalid request body\"}");
  }
  const ArRequestHandlerFunction setParametersHandler =
      [this](AsyncWebServerRequest* request) {
        if (!request->hasArg("plain")) {
          returnInvalidRequest(request);
          return;
        }
        auto gain =
            control::Gain::fromJsonString(request->arg("plain").c_str());
        controller->updateGain(gain);
        request->send(201, "application/json", "");
      };

  const ArRequestHandlerFunction getTargetAngleHandler =
      [this](AsyncWebServerRequest* request) {
        auto angle = controller->getTargetAngle();
        request->send(200, "application/json",
                      "{\"angle\":\"" + String(angle) + "\"}");
      };

  const ArRequestHandlerFunction setTargetAngleHandler =
      [this](AsyncWebServerRequest* request) {
        if (!request->hasArg("plain")) {
          returnInvalidRequest(request);
          return;
        }
        JsonDocument doc;
        // todo: error handling
        deserializeJson(doc, request->arg("plain"));
        auto res = controller->updateTargetAngle(doc["angle"]);
        if (res < 0) {
          request->send(400, "application/json",
                        "{\"message\":\"invalid angle value\"}");
          return;
        }
        request->send(201, "application/json", "");
      };

  void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                        AwsEventType type, void* arg, uint8_t* data,
                        size_t len) {
    // DO NOTHING
    return;
  }

  void broadCast() {
    auto xLastWakeTime = xTaskGetTickCount();
    const auto xFrequency = pdMS_TO_TICKS(20);
    while (1) {
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      WsResponse response(controller->getCurrentAngle().x,
                          controller->getInput(), controller->getTargetAngle(),
                          xLastWakeTime);

      auto jsonResponse = response.toJson();
      ws->textAll(jsonResponse.c_str());
    }
  }

  static void broadCastWrapper(void* arg) {
    static_cast<MyServer*>(arg)->broadCast();
  };

  void start() {
    server->begin();
    xTaskCreate(broadCastWrapper, "broadcast data",
                CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY,
                &broadcastTaskHandle);
  }

 public:
  MyServer(std::shared_ptr<Controller> controller, int port = 8080)
      : controller(controller) {
    server.reset(new AsyncWebServer(port));
    server->on("/controller/parameters", HTTP_GET, getParametersHandler);
    server->on("/controller/parameters", HTTP_POST, setParametersHandler);
    server->on("/controller/target-angle", HTTP_GET, getTargetAngleHandler);
    server->on("/controller/target-angle", HTTP_POST, setTargetAngleHandler);
    ws.reset(new AsyncWebSocket("/ws"));
    ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                       AwsEventType type, void* arg, uint8_t* data,
                       size_t len) {
      this->onWebSocketEvent(server, client, type, arg, data, len);
    });
    server->addHandler(ws.get());
    start();
  };
  ~MyServer() {
    if (serverTaskHandle) {
      vTaskDelete(serverTaskHandle);
    }
    if (wsTaskHandle) {
      vTaskDelete(wsTaskHandle);
    }
    if (broadcastTaskHandle) {
      vTaskDelete(broadcastTaskHandle);
    }
    ws->closeAll();
    server->end();
  };
};

#endif
