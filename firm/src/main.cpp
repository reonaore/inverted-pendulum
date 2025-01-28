#include <Arduino.h>
#include <Kalman.h>
#include <M5StickCPlus2.h>
#include <MadgwickAHRS.h>

#include <Imu.hpp>
#include <Motor.hpp>
#include <PwmController.hpp>

std::unique_ptr<Motor> motor;
std::unique_ptr<Imu> imu;

void vUpdateDisplay(void *pvParameters) {
  char str[200];
  M5.Display.setRotation(1);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setFont(&fonts::Orbitron_Light_24);
  M5.Display.setTextSize(0.5);
  for (;;) {
    auto gyro = imu->getGyro();
    auto angle = imu->getAngle();
    auto angleMadgwick = imu->getAngleMadgwick();
    auto accel = imu->getAccel();
    sprintf(str,
            "%6.2f  %6.2f  %6.2f\t\n"
            "\t  \t o/s \n\n"
            "%6.2f  %6.2f  %6.2f\t \n"
            "\t  \t o/ madgewick\n\n"
            "%6.2f  %6.2f  %6.2f\t \n"
            "\t  \t o/ kalman",
            gyro.x, gyro.y, gyro.z, angleMadgwick.x, angleMadgwick.y,
            angleMadgwick.z, angle.x, angle.y, 0);
    M5.Display.setCursor(0, 10);
    M5.Display.print(str);
    delay(500);
  }
}

void setup() {
  auto config = M5.config();
  config.serial_baudrate = 115200;
  M5.begin(M5.config());
  // motor.reset(Motor::create(IoPins::IO0, IoPins::IO26));
  imu.reset(new Imu());
  xTaskCreateUniversal(vUpdateDisplay, "update display task",
                       CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL,
                       tskIDLE_PRIORITY + 4, NULL, 1);
}

void loop() {}

double v = 0.0;
bool started = false;
double vDiff = 0.1;

void motorTest() {
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
