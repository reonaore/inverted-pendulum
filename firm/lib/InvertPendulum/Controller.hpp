#ifndef _INVERT_PENDULUM_CONTROLLER_HPP_
#define _INVERT_PENDULUM_CONTROLLER_HPP_

#include <ArduinoJson.h>
#include <M5StickCPlus2.h>

#include <Imu.hpp>
#include <Motor.hpp>

namespace control {
class Gain {
 public:
  double kp;
  double ki;
  double kd;

  const String toJson() {
    JsonDocument doc;
    String str;
    doc["kp"] = kp;
    doc["ki"] = ki;
    doc["kd"] = kd;
    serializeJson(doc, str);
    return str;
  }

  static Gain fromJsonString(const char* str) {
    JsonDocument doc;
    // todo: error handling
    deserializeJson(doc, str);
    return Gain(doc["kp"], doc["ki"], doc["kd"]);
  }

  Gain(double kp = 0, double ki = 0, double kd = 0) : kp(kp), ki(ki), kd(kd) {};
  ~Gain() {};
};

}  // namespace control

using namespace control;

class Controller {
 private:
  /* data */
  Gain gain = Gain(0.3, 0.0001, 0.1);

  static constexpr double taskPeriodMs = 20;
  static constexpr double targetAngle = 90.0;
  std::unique_ptr<Motor> motor;
  std::unique_ptr<Imu> imu;

  xTaskHandle taskHandle = 0;

  double ePrev = 0.0;
  double eSum = 0.0;
  boolean stopped = false;
  double inputV = 0.0;
  void stopControl() {
    ePrev = 0;
    eSum = 0;
    stopped = true;
    inputV = 0.0;
    motor->setVoltage(inputV);
  }
  // for graph
  imu_3d_t currentAngle;
  imu_3d_t getAngle() {
    currentAngle = imu->getAngle();
    return currentAngle;
  }
  double cap(double input, double cap = 5.0) {
    if (input > cap) {
      return cap;
    } else if (input < -cap) {
      return -cap;
    }
    return input;
  }
  void mainLoop() {
    auto xLastWakeTime = xTaskGetTickCount();
    while (1) {
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPeriodMs));
      auto e = targetAngle - getAngle().x;
      if (abs(e) > 20) {
        stopControl();
      } else if (abs(e) < 2) {
        stopped = false;
      }
      if (stopped) {
        continue;
      }
      eSum += e;
      // eSum = cap(eSum, 1000);
      auto pV = e * gain.kp;
      auto iV = eSum * gain.ki;
      auto dV = (e - ePrev) / taskPeriodMs * gain.kd;
      inputV = cap(pV) + cap(iV) + cap(dV);
      motor->setVoltage(inputV);
      ePrev = e;
    }
  }
  static void taskEntry(void* arg) {
    static_cast<Controller*>(arg)->mainLoop();
  }
  void createTask() {
    xTaskCreate(taskEntry, "main control loop", CONFIG_ARDUINO_LOOP_STACK_SIZE,
                this, tskIDLE_PRIORITY + 2, &taskHandle);
  }

 public:
  void updateGain(Gain newOne) { this->gain = newOne; }
  const Gain getGain() { return this->gain; }
  const double getInput() { return inputV; };
  const imu_3d_t getCurrentAngle() { return currentAngle; }
  Controller(Motor* motor, Imu* imu) : imu(imu), motor(motor) { createTask(); };
  ~Controller() {
    if (taskHandle) {
      vTaskDelete(taskHandle);
    }
  };
};

#endif
