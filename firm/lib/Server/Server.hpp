#ifndef _SERVER_HPP_
#define _SERVER_HPP_
#include <WebServer.h>

#include <Controller.hpp>
class MyServer {
 private:
  /* data */
  xTaskHandle taskHandle = 0;
  std::unique_ptr<WebServer> server;
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
    auto angle = controller->updateTargetAngle(doc["angle"]);
    server->send(200, "application/json",
                 "{\"angle\":\"" + String(angle) + "\"}");
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
  void start() {
    server->begin();
    xTaskCreate(listenWrapper, "server listen", CONFIG_ARDUINO_LOOP_STACK_SIZE,
                this, tskIDLE_PRIORITY + 1, &taskHandle);
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
    start();
  };
  ~MyServer() {
    if (taskHandle) {
      vTaskDelete(taskHandle);
    }
    server->stop();
  };
};

#endif
