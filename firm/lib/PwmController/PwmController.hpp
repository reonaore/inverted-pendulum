#ifndef _PWM_CONTROLLER_HPP_
#define _PWM_CONTROLLER_HPP_

#include <M5StickCPlus2.h>

#include <hardware.hpp>
#include <map>

#include "PwmChannel.hpp"
#include "pin.hpp"

enum PwmResolutionBits {
  bits_10 = 10,
  bits_11 = 11,
  bits_12 = 12,
};

const std::map<PwmResolutionBits, uint16_t> resolutionByBit = {
    {bits_10, 1024}, {bits_11, 2048}, {bits_12, 4096}};
const std::map<PwmResolutionBits, uint32_t> frequencyByBit = {
    {bits_10, 78125}, {bits_11, 39062}, {bits_12, 19531}};

class PwmController {
 private:
  /* data */
  const uint16_t resolution;
  const uint32_t freq;
  const uint8_t pwmCh;
  const PwmResolutionBits bits;
  const uint8_t pinNo;
  const double dutyStep;

  void begin() {
    pinMode(pinNo, OUTPUT);
    ledcSetup(pwmCh, freq, bits);
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
  PwmController(PwmChannels pwmCh, IoPins pinNo,
                PwmResolutionBits bits = bits_10)
      : resolution(resolutionByBit.at(bits)),
        freq(frequencyByBit.at(bits)),
        pwmCh(pwmCh),
        bits(bits),
        pinNo(pinNo),
        dutyStep(resolution / VCC) {
    begin();
  };
  ~PwmController() { setDutyByVoltage(0); };
};

#endif
