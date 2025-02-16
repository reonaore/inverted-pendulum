#ifndef WS_RESPONSE_HPP
#define WS_RESPONSE_HPP

#include <ArduinoJson.h>

#include <string>

class WsResponse {
 public:
  double angle;
  double input;
  double target;
  uint32_t time;

  WsResponse(double angle, double input, double target, uint32_t time)
      : angle(angle), input(input), target(target), time(time) {}

  const std::string toJson() {
    JsonDocument doc;
    doc["angle"] = angle;
    doc["input"] = input;
    doc["target"] = target;
    doc["time"] = time;

    std::string output;
    serializeJson(doc, output);
    return output;
  }
};
;

#endif  // WS_RESPONSE_HPP
