#include <Arduino.h>
#include <Kalman.h>
#include <M5StickCPlus2.h>
#include <MadgwickAHRS.h>

#include <Motor.hpp>
#include <PwmController.hpp>

std::unique_ptr<Motor> motor;
Madgwick filter;
m5::IMU_Class::imu_3d_t kalmanAngle;

void vUpdateImu(void *pvParameters) {
  TickType_t xLastWakeTime;
  M5.Imu.begin();
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    M5.Imu.update();
  }
}

void vUpdateImuMadgwick(void *pvParameters) {
  TickType_t xLastWakeTime;

  // Initialize the last wake time
  filter.begin(25);  // TODO
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(40));
    auto data = M5.Imu.getImuData();
    filter.updateIMU(data.gyro.x, data.gyro.y, data.gyro.z, data.accel.x,
                     data.accel.y, data.accel.z);
  }
}

m5::imu_3d_t calcDegreeFromAccel(m5::imu_3d_t accel) {
  m5::imu_3d_t res;
  res.x = atan2(accel.y, accel.z) * 180 / PI;
  res.y =
      atan2(-accel.x, sqrt(accel.y * accel.y + accel.z * accel.z)) * 180 / PI;
  return res;
}

void vUpdateImuKalman(void *pvParameters) {
  TickType_t xLastWakeTime;
  std::unique_ptr<Kalman> kalmanFilterX, kalmanFilterY;

  auto dt = 0.005;  // 5msec

  // Initialize the last wake time
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
    auto data = M5.Imu.getImuData();
    auto angle = calcDegreeFromAccel(data.accel);
    if (!kalmanFilterX) {
      kalmanFilterX.reset(new Kalman());
      kalmanFilterX->setAngle(angle.x);
      kalmanFilterY.reset(new Kalman());
      kalmanFilterY->setAngle(angle.y);
    } else {
      kalmanAngle.x = kalmanFilterX->getAngle(angle.x, data.gyro.x, dt);
      kalmanAngle.y = kalmanFilterY->getAngle(angle.y, data.gyro.y, dt);
    }
  }
}

void vUpdateDisplay(void *pvParameters) {
  char str[200];
  M5.Display.setRotation(1);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setFont(&fonts::Orbitron_Light_24);
  M5.Display.setTextSize(0.5);
  for (;;) {
    auto data = M5.Imu.getImuData();
    sprintf(str,
            "%6.2f  %6.2f  %6.2f\t\n"
            "\t  \t o/s \n\n"
            "%6.2f  %6.2f  %6.2f\t \n"
            "\t  \t o/ madgewick\n\n"
            "%6.2f  %6.2f  %6.2f\t \n"
            "\t  \t o/ kalman",
            data.gyro.x, data.gyro.y, data.gyro.z, filter.getRoll(),
            filter.getPitch(), filter.getYaw(), kalmanAngle.x, kalmanAngle.y,
            kalmanAngle.z);
    M5.Display.setCursor(0, 10);
    M5.Display.print(str);
    delay(500);
  }
}

void setup() {
  auto config = M5.config();
  config.serial_baudrate = 115200;
  M5.begin(M5.config());
  motor.reset(Motor::create(IoPins::IO0, IoPins::IO26));
  xTaskCreateUniversal(vUpdateImu, "update imu task",
                       CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL,
                       tskIDLE_PRIORITY + 2, NULL, 1);
  xTaskCreateUniversal(vUpdateImuMadgwick, "update imu task by madgwick",
                       CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL,
                       tskIDLE_PRIORITY + 1, NULL, 1);
  xTaskCreateUniversal(vUpdateImuKalman, "update imu task by kalman",
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
