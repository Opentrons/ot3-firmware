#pragma once

#include <map>

namespace sensor_simulator {
class SensorType {
  public:
    SensorType() : DEVICE_ID{0}, ADDRESS{0}, REGISTER_MAP{}{}
    uint16_t DEVICE_ID;
    uint16_t ADDRESS;
    std::map<uint8_t, uint32_t> REGISTER_MAP;
};
}  // namespace sensor_simulator
