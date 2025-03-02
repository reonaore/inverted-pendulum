#ifndef _OTA_HANDLER_HPP_
#define _OTA_HANDLER_HPP_

#include <ArduinoOTA.h>
#include <WiFi.h>

class OtaHandlerClass {
 private:
  /* data */

  xTaskHandle taskHandle = 0;
  bool isStarted = false;

 public:
  OtaHandlerClass() {
    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.setMdnsEnabled(true);
    ArduinoOTA.setHostname(HOST_NAME);
  };
  ~OtaHandlerClass() {
    if (taskHandle) {
      vTaskDelete(taskHandle);
    }
    ArduinoOTA.end();
  };

  void startHandling() {
    if (isStarted) {
      return;
    }
    ArduinoOTA.begin();
    xTaskCreate(
        [](void *arg) {
          while (true) {
            ArduinoOTA.handle();
            delay(1000);
          }
        },
        "OTA Handler", CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY,
        &taskHandle);
    isStarted = true;
  }
};

OtaHandlerClass OtaHandler;

#endif
