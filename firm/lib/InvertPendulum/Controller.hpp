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
  static Gain fromJson(JsonDocument doc) {
    // todo: error handling
    return Gain(doc["kp"], doc["ki"], doc["kd"]);
  }

  static Gain fromJsonString(const char* str) {
    JsonDocument doc;
    // todo: error handling
    deserializeJson(doc, str);
    return fromJson(doc);
  }
  boolean isZero() { return kp == 0 && kd == 0 && ki == 0; }

  Gain(double kp = 0, double ki = 0, double kd = 0) : kp(kp), ki(ki), kd(kd) {};
  ~Gain() {};
};

}  // namespace control

using namespace control;

class Controller {
 private:
  /* data */
  static constexpr double taskPeriodMs = 20;
  static constexpr double taskPeriodSec = 0.02;

  Gain gain = Gain(0.24, 0.02, 0.003);
  double targetAngle = 90.0;

  std::unique_ptr<Motor> motor;
  std::unique_ptr<Imu> imu;

  xTaskHandle taskHandle = 0;
  double ePrev = 0.0;
  double eSum = 0.0;
  boolean stopped = false;
  double inputV = 0.0;

  void stopControl() {
    inputV = 0.0;
    ePrev = 0;
    eSum = 0;
    motor->setVoltage(inputV);
    stopped = true;
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
        continue;
      } else if (abs(e) < 1) {
        stopped = false;
      }
      if (stopped) {
        continue;
      }
      eSum += e * taskPeriodSec;
      auto pV = cap(e * gain.kp, VCC);
      auto iV = cap(eSum * gain.ki, VCC);
      auto dV = cap((((e - ePrev) / taskPeriodSec) * gain.kd), VCC);
      if (abs(iV) == VCC) {
        eSum = 0;
      }
      inputV = cap(pV + iV + dV);
      motor->setVoltage(inputV);
      ePrev = e;
    }
  }
  static void taskEntry(void* arg) {
    static_cast<Controller*>(arg)->mainLoop();
  }
  void start() {
    xTaskCreate(taskEntry, "main control loop", CONFIG_ARDUINO_LOOP_STACK_SIZE,
                this, tskIDLE_PRIORITY + 4, &taskHandle);
  }

 public:
  void updateGain(Gain newOne) { this->gain = newOne; }
  const Gain getGain() { return this->gain; }
  const double getInput() { return inputV; };
  const imu_3d_t getCurrentAngle() { return currentAngle; }
  const double getTargetAngle() { return targetAngle; }
  int updateTargetAngle(double angle) {
    if (angle > 110 || angle < 70) {
      return -1;
    }
    targetAngle = angle;
    return 0;
  }
  Controller(Motor* motor, Imu* imu) : imu(imu), motor(motor) { start(); };
  ~Controller() {
    if (taskHandle) {
      vTaskDelete(taskHandle);
    }
  };
};

#endif
