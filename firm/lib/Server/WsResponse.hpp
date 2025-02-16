#ifndef WS_RESPONSE_HPP
#define WS_RESPONSE_HPP

#include <ArduinoJson.h>

#include <string>

class WsResponse {
 public:
  double angle;
  double input;
  double target;

  WsResponse(double angle, double input, double target)
      : angle(angle), input(input), target(target) {}

  const std::string toJson() {
    JsonDocument doc;
    doc["angle"] = angle;
    doc["input"] = input;
    doc["target"] = target;

    std::string output;
    serializeJson(doc, output);
    return output;
  }
};

#endif  // WS_RESPONSE_HPP
