#pragma once
#include "eeprom/simulation/eeprom.hpp"
#include "i2c/simulation/i2c_sim.hpp"
/*
** Simulator-specific interfaces
*/

namespace interfaces {
void initialize_sim(int argc, char** argv);
auto get_sim_eeprom() -> std::shared_ptr<eeprom::simulator::EEProm>;
auto get_sim_i2c2() -> std::shared_ptr<i2c::hardware::SimI2C>;
};  // namespace interfaces
