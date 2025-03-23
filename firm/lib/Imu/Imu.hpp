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
  xTaskHandle taskHandle = 0;
  imu_3d_t angle = {0};
  imu_3d_t gyroOffset = {0};
  imu_data_t data = {0, {0}};

  static imu_3d_t calcDegreeFromAccel(imu_3d_t accel) {
    imu_3d_t res = {0};
    res.x = atan2(accel.y, accel.z) * RAD_TO_DEG;
    res.y = atan(-accel.x / sqrt(accel.y * accel.y + accel.z * accel.z)) *
            RAD_TO_DEG;
    return res;
  }

  void createTask() {
    xTaskCreate(taskEntry, "update imu task kalman",
                CONFIG_ARDUINO_LOOP_STACK_SIZE, this, tskIDLE_PRIORITY + 1,
                &taskHandle);
  }

  void calibration() {
    auto n = 100;
    auto i = 0;
    while (i != n) {
      if (!M5.Imu.update()) {
        continue;
      }
      auto current = M5.Imu.getImuData();
      gyroOffset.x += current.gyro.x;
      gyroOffset.y += current.gyro.y;
      gyroOffset.z += current.gyro.z;
      i++;
    }
    gyroOffset.x /= n;
    gyroOffset.y /= n;
    gyroOffset.z /= n;
  }

  void vUpdateImuMadgwick() {
    auto sampleFreq = 25.0;
    auto taskPeriodMs = 40;

    filter.begin(sampleFreq);
    // Initialize the last wake time
    auto xLastWakeTime = xTaskGetTickCount();
    while (1) {
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPeriodMs));
      updateImuData();
      filter.updateIMU(data.gyro.x, data.gyro.y, data.gyro.z, data.accel.x,
                       data.accel.y, data.accel.z);
      this->angle.x = filter.getRoll();
      this->angle.y = filter.getPitch();
    }
  }

  void updateImuData() {
    M5.Imu.update();
    auto current = M5.Imu.getImuData();
    data.gyro.x = current.gyro.x - gyroOffset.x;
    data.gyro.y = current.gyro.y - gyroOffset.y;
    data.gyro.z = current.gyro.z - gyroOffset.z;
    data.accel.x = current.accel.x * 0.1 + data.accel.x * 0.9;
    data.accel.y = current.accel.y * 0.1 + data.accel.y * 0.9;
    data.accel.z = current.accel.z * 0.1 + data.accel.z * 0.9;
    return;
  }

  void vUpdateImuKalman() {
    auto xLastWakeTime = xTaskGetTickCount();
    auto initialized = false;
    while (1) {
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPeriodMs));
      updateImuData();
      auto calculatedAngle = calcDegreeFromAccel(data.accel);
      if (!initialized) {
        kalmanFilterX.setAngle(calculatedAngle.x);
        kalmanFilterY.setAngle(calculatedAngle.y);
        initialized = !initialized;
        continue;
      }
      this->angle.x =
          kalmanFilterX.getAngle(calculatedAngle.x, data.gyro.x, dt);
      this->angle.y =
          kalmanFilterY.getAngle(calculatedAngle.y, data.gyro.y, dt);
    }
  }

  static void taskEntry(void* arg) {
    static_cast<Imu*>(arg)->vUpdateImuKalman();
    // static_cast<Imu*>(arg)->vUpdateImuMadgwick();
  }

 public:
  Imu() {};
  ~Imu() { end(); }
  void begin() {
    M5.Imu.begin();
    createTask();
    delay(taskPeriodMs * 2);
    calibration();
  }
  void end() {
    if (taskHandle) {
      vTaskDelete(taskHandle);
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
  imu_3d_t getGyro() { return data.gyro; }
  imu_3d_t getAccel() { return data.accel; }
};

#endif
