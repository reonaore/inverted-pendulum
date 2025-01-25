#include <Arduino.h>
#include <M5StickCPlus2.h>

#include <Motor.hpp>
#include <PwmController.hpp>

std::unique_ptr<Motor> motor;
// std::unique_ptr<PwmController> pwmA;

void setup() {
  M5.begin(M5.config());
  motor.reset(Motor::create(IoPins::IO0, IoPins::IO26));
  // pwmA.reset(new PwmController(PwmChannels::CH0, IoPins::IO0));

  M5.Display.setRotation(1);
  StickCP2.Display.setTextDatum(middle_center);
  StickCP2.Display.setFont(&fonts::Orbitron_Light_24);
  M5.Display.setTextSize(0.5);
}

double v = 0.0;
bool started = false;
double vDiff = 0.1;

void loop() {
  M5.update();
  M5.Display.setCursor(10, 30);
  M5.Display.printf("volt = %03.2f\t ", v);
  if (M5.BtnA.wasPressed()) {
    started = !started;
  }
  if (started) {
    v += vDiff;
  } else {
    v = 0.0;
    // return;
  }
  if (v >= 5.0) {
    v = 0.0;
    vDiff *= -1.0;
  }
  if (v <= -5.0) {
    v = 0.0;
    vDiff *= -1.0;
  }
  auto duty = motor.get()->setVoltage(v);
  M5.Display.setCursor(10, 70);
  M5.Display.printf("duty = %03d\t ", duty);
  delay(500);
}
