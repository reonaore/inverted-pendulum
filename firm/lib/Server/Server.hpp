#ifndef _SERVER_HPP_
#define _SERVER_HPP_
#include <WebServer.h>
#include <WebSocketsServer.h>

#include <Controller.hpp>

#include "WsResponse.hpp"
class MyServer {
 private:
  /* data */
  xTaskHandle serverTaskHandle = 0;
  xTaskHandle wsTaskHandle = 0;
  xTaskHandle broadcastTaskHandle = 0;
  std::unique_ptr<WebServer> server;
  std::unique_ptr<WebSocketsServer> ws;
  std::shared_ptr<Controller> controller;

  void getParameterHandler() {
    auto gain = controller->getGain();
    server->send(200, "application/json", gain.toJson());
  };

  void returnInvalidRequest() {
    server->send(400, "application/json",
                 "{\"message\",\"invalid request body\"}");
  }

  void setParameterHandler() {
    if (!server->hasArg("plain")) {
      returnInvalidRequest();
      return;
    }
    auto gain = control::Gain::fromJsonString(server->arg("plain").c_str());
    controller->updateGain(gain);
    server->send(201, "application/json", "");
  }

  void getTargetAngleHandler() {
    auto angle = controller->getTargetAngle();
    server->send(200, "application/json",
                 "{\"angle\":\"" + String(angle) + "\"}");
  }

  void setTargetAngleHandler() {
    if (!server->hasArg("plain")) {
      returnInvalidRequest();
      return;
    }
    JsonDocument doc;
    // todo: error handling
    deserializeJson(doc, server->arg("plain"));
    auto res = controller->updateTargetAngle(doc["angle"]);
    if (res < 0) {
      server->send(400, "application/json",
                   "{\"message\":\"invalid angle value\"}");
      return;
    }
    server->send(201, "application/json", "");
  }

  void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload,
                      size_t length) {
    // DO NOTHING
    return;
  }

  void broadCast() {
    auto xLastWakeTime = xTaskGetTickCount();
    const auto xFrequency = pdMS_TO_TICKS(20);

    while (1) {
      WsResponse response(controller->getCurrentAngle().x,
                          controller->getInput(), controller->getTargetAngle());

      auto jsonResponse = response.toJson();
      ws->broadcastTXT(jsonResponse.c_str());
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
  }

  void listenWs() {
    while (1) {
      ws->loop();
      yield();
    }
  }

  void listen() {
    while (1) {
      server->handleClient();
      yield();
    }
  }

  static void listenWrapper(void* arg) {
    static_cast<MyServer*>(arg)->listen();
  };

  static void listenWsWrapper(void* arg) {
    static_cast<MyServer*>(arg)->listenWs();
  };

  static void broadCastWrapper(void* arg) {
    static_cast<MyServer*>(arg)->broadCast();
  };

  void start() {
    server->begin();
    xTaskCreate(listenWrapper, "server listen", CONFIG_ARDUINO_LOOP_STACK_SIZE,
                this, tskIDLE_PRIORITY, &serverTaskHandle);
    ws->begin();
    xTaskCreate(listenWsWrapper, "ws server listen",
                CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY,
                &wsTaskHandle);
    xTaskCreate(broadCastWrapper, "broadcast data",
                CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY,
                &broadcastTaskHandle);
  }

 public:
  MyServer(std::shared_ptr<Controller> controller, int port = 8080)
      : controller(controller) {
    server.reset(new WebServer(port));
    server->on("/controller/parameters", HTTP_GET,
               std::bind(&MyServer::getParameterHandler, this));
    server->on("/controller/parameters", HTTP_POST,
               std::bind(&MyServer::setParameterHandler, this));
    server->on("/controller/target-angle", HTTP_GET,
               std::bind(&MyServer::getTargetAngleHandler, this));
    server->on("/controller/target-angle", HTTP_POST,
               std::bind(&MyServer::setTargetAngleHandler, this));
    ws.reset(new WebSocketsServer(port, "ws", "ws"));
    ws->onEvent(std::bind(&MyServer::webSocketEvent, this,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4));
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
    server->stop();
    ws->close();
  };
};

#endif
