#ifndef _MOTOR_HPP_
#define _MOTOR_HPP_
#include <PwmController.hpp>
#include <hardware.hpp>

class Motor {
 private:
  /* data */
  static constexpr double offsetVoltage = 0.7;
  PwmController &front;
  PwmController &back;

 public:
  Motor() = delete;
  Motor(PwmController &front, PwmController &back) : front(front), back(back) {
    setVoltage(0);
  };
  double setVoltage(double v) {
    if (v != 0) {
      if (v > 0) {
        v += offsetVoltage;
      } else {
        v -= offsetVoltage;
      }
    }
    if (v >= 0) {
      front.setDutyByVoltage(v);
      back.setDutyByVoltage(0);
    } else {
      front.setDutyByVoltage(0);
      back.setDutyByVoltage(v);
    }
    return v;
  }

  void stop() { setVoltage(0); }
  ~Motor() { stop(); };
};
#endif
