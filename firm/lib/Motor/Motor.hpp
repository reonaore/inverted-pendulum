#ifndef _MOTOR_HPP_
#define _MOTOR_HPP_
#include <PwmController.hpp>
#include <hardware.hpp>

class Motor {
 private:
  /* data */
  static const uint8_t resolutionBit = 10;  // bit
  static const uint16_t resolution = 1024;
  static const uint32_t pwmFrequency = 78125;  // Hz
  static constexpr double dutyStep = (resolution / VCC);
  const std::unique_ptr<PwmController> front;
  const std::unique_ptr<PwmController> back;

 public:
  static uint16_t duty(double v) {
    auto duty = abs(v) * dutyStep;
    if (duty >= (double)resolution) {
      duty = (double)resolution;
    }
    return round(duty);
  }

  Motor() = delete;
  Motor(PwmController *front, PwmController *back) : front(front), back(back) {
    this->setVoltage(0);
  };
  static Motor *create(IoPins front, IoPins back) {
    auto f =
        new PwmController(PwmChannels::CH0, front, pwmFrequency, resolutionBit);
    auto b =
        new PwmController(PwmChannels::CH1, back, pwmFrequency, resolutionBit);
    return new Motor(f, b);
  }
  uint16_t setVoltage(double v) {
    auto d = duty(v);
    if (v >= 0) {
      front->setDuty(d);
      back->setDuty(0);
    } else {
      front->setDuty(0);
      back->setDuty(d);
    }
    return d;
  }
  ~Motor() { this->setVoltage(0); };
};
#endif
