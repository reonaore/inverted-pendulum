#ifndef _SERVER_HPP_
#define _SERVER_HPP_
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

#include <Controller.hpp>

#include "WsResponse.hpp"
class MyServer {
 private:
  /* data */
  xTaskHandle serverTaskHandle = 0;
  xTaskHandle wsTaskHandle = 0;
  xTaskHandle broadcastTaskHandle = 0;
  Controller& controller;
  std::unique_ptr<AsyncWebServer> server;
  std::unique_ptr<AsyncWebSocket> ws;

  const ArRequestHandlerFunction getParametersHandler =
      [this](AsyncWebServerRequest* request) {
        auto gain = controller.getGain();
        request->send(200, "application/json", gain.toJson());
      };

  void returnInvalidRequest(AsyncWebServerRequest* request) {
    request->send(400, "application/json",
                  "{\"message\",\"invalid request body\"}");
  }
  const ArJsonRequestHandlerFunction setParametersHandler =
      [this](AsyncWebServerRequest* request, JsonVariant& json) {
        if (!json.is<JsonObject>()) {
          returnInvalidRequest(request);
          return;
        }
        auto gain = control::Gain::fromJson(json.as<JsonObject>());
        if (gain.isZero()) {
          returnInvalidRequest(request);
          return;
        }
        controller.updateGain(gain);
        request->send(204, "application/json", "");
      };

  const ArRequestHandlerFunction getTargetAngleHandler =
      [this](AsyncWebServerRequest* request) {
        auto angle = controller.getTargetAngle();
        request->send(200, "application/json",
                      "{\"angle\":" + String(angle) + "}");
      };

  const ArJsonRequestHandlerFunction setTargetAngleHandler =
      [this](AsyncWebServerRequest* request, JsonVariant& json) {
        if (!json.is<JsonObject>()) {
          returnInvalidRequest(request);
          return;
        }
        auto doc = json.as<JsonObject>();
        auto res = controller.updateTargetAngle(doc["angle"]);
        if (res < 0) {
          request->send(400, "application/json",
                        "{\"message\":\"invalid angle value\"}");
          return;
        }
        request->send(204, "application/json", "");
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
      WsResponse response(controller.getCurrentAngle().x, controller.getInput(),
                          controller.getTargetAngle(), xLastWakeTime);

      auto jsonResponse = response.toJson();
      ws->textAll(jsonResponse.c_str());
    }
  }

  static void broadCastWrapper(void* arg) {
    static_cast<MyServer*>(arg)->broadCast();
  };

  // todo: remove pointer at destructor
  AsyncCallbackJsonWebHandler* on(const char* uri,
                                  WebRequestMethodComposite method,
                                  ArJsonRequestHandlerFunction onRequest) {
    auto h = new AsyncCallbackJsonWebHandler(uri, onRequest);
    h->setMethod(HTTP_POST);
    return h;
  }

  AsyncWebSocket* createWebSocket() {
    auto ws = new AsyncWebSocket("/ws");
    ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                       AwsEventType type, void* arg, uint8_t* data,
                       size_t len) {
      this->onWebSocketEvent(server, client, type, arg, data, len);
    });
    return ws;
  }

  AsyncWebServer* createAsyncWebServer(int port) {
    auto server = new AsyncWebServer(port);
    server->on("/controller/parameters", HTTP_GET, getParametersHandler);
    server->addHandler(
        on("/controller/parameters", HTTP_POST, setParametersHandler));
    server->on("/controller/target-angle", HTTP_GET, getTargetAngleHandler);
    server->addHandler(
        on("/controller/target-angle", HTTP_POST, setTargetAngleHandler));
    return server;
  }

 public:
  void begin() {
    controller.begin();
    server->begin();
    xTaskCreate(broadCastWrapper, "broadcast data",
                CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY,
                &broadcastTaskHandle);
  }
  MyServer() = delete;
  MyServer(Controller& controller, int port = 8080) : controller(controller) {
    ws.reset(createWebSocket());
    server.reset(createAsyncWebServer(port));

    server->addHandler(ws.get());
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
