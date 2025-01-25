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

  void setup() {
    pinMode(pinNo, OUTPUT);
    ledcSetup(pwmCh, freq, resolution);
    ledcAttachPin(pinNo, pwmCh);
  }

 public:
  static const uint32_t defaultHz = 312500;
  static const uint32_t defaultResolution = 8;  // bit
  PwmController() = delete;
  PwmController(PwmChannels pwmCh, IoPins pinNo, uint32_t freq = defaultHz,
                uint32_t resolution = defaultResolution)
      : freq(freq), pwmCh(pwmCh), resolution(resolution), pinNo(pinNo) {
    setup();
  };
  ~PwmController() { setDuty(0); };

  void setDuty(uint32_t duty) { ledcWrite(pwmCh, duty); };
};

#endif
