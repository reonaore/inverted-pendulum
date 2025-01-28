#ifndef _INVERT_PENDULUM_CONTROLLER_HPP_
#define _INVERT_PENDULUM_CONTROLLER_HPP_

#include <M5StickCPlus2.h>

#include <Imu.hpp>
#include <Motor.hpp>

class Controller {
 private:
  /* data */
  const double kp = 0.1;
  const double ki = 0.01;
  const double kd = 0.1;

  static constexpr double taskPeriodMs = 5;
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
  void mainLoop() {
    auto xLastWakeTime = xTaskGetTickCount();
    while (1) {
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPeriodMs));
      auto e = targetAngle - imu->getAngle().x;
      if (abs(e) > 20) {
        stopControl();
      } else {
        stopped = false;
      }
      if (stopped) {
        continue;
      }
      eSum += e;
      inputV = ((e - ePrev) / taskPeriodMs * kd) + (eSum * ki) + (e * kp);
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
  double getInput() { return inputV; };
  Controller(Motor* motor, Imu* imu) : imu(imu), motor(motor) { createTask(); };
  ~Controller() {
    if (taskHandle) {
      vTaskDelete(taskHandle);
    }
  };
};

#endif
