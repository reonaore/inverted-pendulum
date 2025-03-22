#include <Arduino.h>
#include <Kalman.h>
#include <M5StickCPlus2.h>
#include <MadgwickAHRS.h>
#include <SPIFFS.h>

#include <Controller.hpp>
#include <Imu.hpp>
#include <Motor.hpp>
#include <OtaHandler.hpp>
#include <PwmController.hpp>
#include <Server.hpp>
#include <wifiConfig.hpp>

PwmController front(PwmChannels::CH0, IoPins::IO0);
PwmController back(PwmChannels::CH1, IoPins::IO26);
Motor motor(front, back);
Imu imu = Imu();
Controller ctrl(motor, imu);
MyServer server(ctrl);
wifiConfig wifi;

void setup() {
  auto config = M5.config();
  config.serial_baudrate = 115200;
  StickCP2.begin(config);
  if (!SPIFFS.begin(true)) {
    throw "SPIFFS mount failed";
  }
  StickCP2.Display.setRotation(3);
  StickCP2.Display.setTextDatum(middle_center);
  StickCP2.Display.setFont(&fonts::Orbitron_Light_24);
  StickCP2.Display.setTextSize(0.6);
  wifiConfig::fromFile().begin();
  StickCP2.Display.setCursor(10, 90);
  StickCP2.Display.println("local IP: " + WiFi.localIP().toString());
  OtaHandler.startHandling();
  server.begin();
}

extern "C" void app_main() {
  // initialize arduino library before we start the tasks
  initArduino();
  setup();
}
