#ifndef _PWM_CONTROLLER_HPP_
#define _PWM_CONTROLLER_HPP_

#include <M5StickCPlus2.h>

#include "PwmChannel.hpp"
#include "pin.hpp"

class PwmController {
 private:
  /* data */
  const uint32_t freq;
  const uint8_t pwmCh;
  const uint32_t resolution;
  const uint8_t pinNo;
  // uint16_t duty = 0;

  void setup() {
    pinMode(pinNo, OUTPUT);
    ledcSetup(pwmCh, freq, resolution);
    ledcAttachPin(pinNo, pwmCh);
  }

 public:
  static const uint32_t defaultHz = 312500;
  static const uint32_t defaultResolution = 8;  // bit
  // static const uint32_t defaultHz = 78125;
  // static const uint32_t defaultResolution = 10;  // bit
  PwmController() = delete;
  PwmController(PwmChannels pwmCh, IoPins pinNo, uint32_t freq = defaultHz,
                uint32_t resolution = defaultResolution)
      : freq(freq), pwmCh(pwmCh), resolution(resolution), pinNo(pinNo) {
    // setup();
    pinMode(pinNo, OUTPUT);
    ledcSetup(pwmCh, freq, resolution);
    ledcAttachPin(pinNo, pwmCh);
  };
  ~PwmController() {
    // this->duty = 0;
    setDuty(0);
  };

  void setDuty(uint32_t duty) {
    // this->duty = duty;
    ledcWrite(pwmCh, duty);
  };
};

#endif
