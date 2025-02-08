#include <Arduino.h>
#include <Kalman.h>
#include <M5StickCPlus2.h>
#include <MadgwickAHRS.h>
#include <SPIFFS.h>

#include <Controller.hpp>
#include <Imu.hpp>
#include <Motor.hpp>
#include <PwmController.hpp>
#include <Server.hpp>
#include <wifiConfig.hpp>

std::shared_ptr<Controller> ctrl;
std::unique_ptr<MyServer> server;
std::unique_ptr<wifiConfig> wifi;

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
  wifi.reset(wifiConfig::fromFile());
  wifi->begin();
  StickCP2.Display.setCursor(10, 90);
  StickCP2.Display.println("local IP: " + WiFi.localIP().toString());
  auto motor = Motor::create(IoPins::IO0, IoPins::IO26);
  auto imu = new Imu();
  ctrl.reset(new Controller(motor, imu));
  server.reset(new MyServer(ctrl));
}

void loop() {
  delay(1000);
  auto v = ctrl->getInput();
  auto angle = ctrl->getCurrentAngle();
  StickCP2.Display.setCursor(0, 10);
  StickCP2.Display.printf(
      "\t v = %5.3f\n"
      "\t angle = %5.3f\n",
      v, angle.x);
}
