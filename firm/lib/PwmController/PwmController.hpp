#ifndef _PWM_CONTROLLER_HPP_
#define _PWM_CONTROLLER_HPP_

#include <M5StickCPlus2.h>

#include <hardware.hpp>

#include "PwmChannel.hpp"
#include "pin.hpp"

class PwmController {
 private:
  static const uint8_t defaultResolutionBit = 10;  // bit
  // todo: set below 2 from bits
  static const uint32_t defaultFrequency = 78125;  // Hz
  static const uint16_t resolution = 1024;

  /* data */
  const uint32_t freq;
  const uint8_t pwmCh;
  const uint8_t resolutionBits;
  const uint8_t pinNo;
  const double dutyStep;

  void begin() {
    pinMode(pinNo, OUTPUT);
    ledcSetup(pwmCh, freq, resolutionBits);
    ledcAttachPin(pinNo, pwmCh);
    setDutyByVoltage(0);
  }

 public:
  void setDutyByVoltage(double v) {
    auto duty = abs(v) * dutyStep;
    if (duty >= (double)resolution) {
      duty = (double)resolution;
    }
    // Flooring the duty
    ledcWrite(pwmCh, duty);
  }
  PwmController() = delete;
  PwmController(PwmChannels pwmCh, IoPins pinNo)
      : freq(defaultFrequency),
        pwmCh(pwmCh),
        resolutionBits(defaultResolutionBit),
        pinNo(pinNo),
        dutyStep(resolution / VCC) {
    begin();
  };
  ~PwmController() { setDutyByVoltage(0); };
};

#endif
