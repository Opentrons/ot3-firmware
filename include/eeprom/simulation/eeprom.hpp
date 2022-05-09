#pragma once

#include "sensors/simulation/sensors.hpp"

namespace eeprom {
namespace simulator {

class EEProm : public sensor_simulator::SensorType {
  public:
    EEProm() {
        DEVICE_ID = 0x0;
        ADDRESS = 0xA0;
        REGISTER_MAP = {{0x0, 0}};
    }
};

}  // namespace simulator
}  // namespace eeprom