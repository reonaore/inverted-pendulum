#ifndef _IMU_HPP_
#define _IMU_HPP_

#include <Kalman.h>
#include <M5StickCPlus2.h>
using namespace m5;

class Imu {
 private:
  /* data */
  static const uint32_t taskPeriodMs = 5;
  static constexpr double dt = 0.005;  // sec
  Kalman kalmanFilterX, kalmanFilterY;
  Madgwick filter;
  xTaskHandle kalmanTaskHandle = 0;
  xTaskHandle madgwickTaskHandle = 0;
  imu_3d_t angle = {0};
  imu_3d_t angleMadgwick = {0};
  imu_3d_t gyro = {0};
  imu_3d_t accel = {0};

  static imu_3d_t calcDegreeFromAccel(imu_3d_t accel) {
    imu_3d_t res = {0};
    res.x = atan2(accel.y, accel.z) * RAD_TO_DEG;
    res.y = atan2(-accel.x, sqrt(accel.y * accel.y + accel.z * accel.z)) *
            RAD_TO_DEG;
    return res;
  }

  void createTask() {
    xTaskCreate(taskEntryKalman, "update imu task kalman",
                CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY + 1,
                &kalmanTaskHandle);
    xTaskCreate(taskEntryMadgwick, "update imu task madgwick",
                CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY + 1,
                &madgwickTaskHandle);
  }

  void vUpdateImuMadgwick() {
    auto sampleFreq = 25.0;
    auto taskPeriodMs = 40;

    filter.begin(sampleFreq);
    // Initialize the last wake time
    auto xLastWakeTime = xTaskGetTickCount();
    while (1) {
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPeriodMs));
      filter.updateIMU(gyro.x, gyro.y, gyro.z, accel.x, accel.y, accel.z);
    }
  }

  void vUpdateImuKalman() {
    auto xLastWakeTime = xTaskGetTickCount();
    auto initialized = false;
    while (1) {
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPeriodMs));
      M5.Imu.update();
      auto data = M5.Imu.getImuData();
      auto angle = calcDegreeFromAccel(data.accel);
      if (!initialized) {
        kalmanFilterX.setAngle(angle.x);
        kalmanFilterY.setAngle(angle.y);
        initialized = !initialized;
      } else {
        this->angle.x = kalmanFilterX.getAngle(angle.x, data.gyro.x, dt);
        this->angle.y = kalmanFilterY.getAngle(angle.y, data.gyro.y, dt);
        this->gyro = data.gyro;
        this->accel = data.accel;
      }
    }
  }

  static void taskEntryKalman(void* arg) {
    static_cast<Imu*>(arg)->vUpdateImuKalman();
  }
  static void taskEntryMadgwick(void* arg) {
    static_cast<Imu*>(arg)->vUpdateImuMadgwick();
  }

 public:
  Imu() {
    M5.Imu.begin();
    createTask();
    delay(taskPeriodMs * 2);
  };
  ~Imu() {
    if (kalmanTaskHandle) {
      vTaskDelete(kalmanTaskHandle);
    }
    if (madgwickTaskHandle) {
      vTaskDelete(madgwickTaskHandle);
    }
  }

  imu_3d_t getAngle() { return angle; }
  imu_3d_t getAngleMadgwick() {
    imu_3d_t res = {0};
    res.x = filter.getRoll();
    res.y = filter.getPitch();
    res.z = filter.getYaw();
    return res;
  }
  imu_3d_t getGyro() { return gyro; }
  imu_3d_t getAccel() { return accel; }
};

#endif
