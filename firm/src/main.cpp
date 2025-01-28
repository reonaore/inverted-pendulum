#include <Arduino.h>
#include <Kalman.h>
#include <M5StickCPlus2.h>
#include <MadgwickAHRS.h>

#include <Controller.hpp>
#include <Imu.hpp>
#include <Motor.hpp>
#include <PwmController.hpp>

std::unique_ptr<Controller> ctrl;

void setup() {
  auto config = M5.config();
  config.serial_baudrate = 115200;
  M5.begin(M5.config());
  StickCP2.Display.setRotation(1);
  StickCP2.Display.setTextDatum(middle_center);
  StickCP2.Display.setFont(&fonts::Orbitron_Light_24);
  StickCP2.Display.setTextSize(1);
  auto motor = Motor::create(IoPins::IO0, IoPins::IO26);
  auto imu = new Imu();
  ctrl.reset(new Controller(motor, imu));
}

void loop() {
  delay(1000);
  auto v = ctrl->getInput();
  StickCP2.Display.setCursor(0, 10);
  StickCP2.Display.printf("v = %f", v);
}
