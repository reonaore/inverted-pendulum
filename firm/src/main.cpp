#include <Arduino.h>
#include <M5StickCPlus2.h>
#include <MadgwickAHRS.h>

#include <Motor.hpp>
#include <PwmController.hpp>

std::unique_ptr<Motor> motor;
Madgwick filter;

void vUpdateImu(void *pvParameters) {
  TickType_t xLastWakeTime;

  // Initialize the last wake time
  filter.begin(25);  // TODO
  M5.Imu.begin();
  xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    if (!M5.Imu.update()) {
      return;
    };
    auto data = M5.Imu.getImuData();
    filter.updateIMU(data.gyro.x, data.gyro.y, data.gyro.z, data.accel.x,
                     data.accel.y, data.accel.z);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(40));
  }
}

void vUpdateDisplay(void *pvParameters) {
  M5.Display.setRotation(1);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setFont(&fonts::Orbitron_Light_24);
  M5.Display.setTextSize(0.5);
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  for (;;) {
    taskENTER_CRITICAL(&mux);
    auto data = M5.Imu.getImuData();
    taskEXIT_CRITICAL(&mux);
    M5.Display.setCursor(0, 10);
    M5.Display.printf("%6.2f  %6.2f  %6.2f\t ", data.gyro.x, data.gyro.y,
                      data.gyro.z);
    M5.Display.setCursor(100, 30);
    M5.Display.print(" o/s");
    M5.Display.setCursor(0, 50);
    M5.Display.printf("%6.2f  %6.2f  %6.2f\t ", filter.getRoll(),
                      filter.getPitch(), filter.getYaw());
    M5.Display.setCursor(100, 70);
    M5.Display.print(" o");
    M5.Display.setCursor(0, 90);
    M5.Display.printf(" %5.2f   %5.2f   %5.2f\t ", data.accel.x, data.accel.y,
                      data.accel.z);
    delay(500);
  }
}

void setup() {
  M5.begin(M5.config());
  motor.reset(Motor::create(IoPins::IO0, IoPins::IO26));
  xTaskCreateUniversal(vUpdateImu, "update imu task",
                       CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL,
                       tskIDLE_PRIORITY + 1, NULL, 1);
  xTaskCreateUniversal(vUpdateDisplay, "update display task",
                       CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL,
                       tskIDLE_PRIORITY + 4, NULL, 1);
}

m5::IMU_Class::imu_3d_t offset;

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
