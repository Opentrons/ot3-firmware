#pragma once

#include "types.hpp"

namespace eeprom {
namespace addresses {

constexpr types::data_length serial_number_length = 8;
constexpr types::address serial_number_address_begin = 0;
constexpr types::address serial_number_address_end = serial_number_address_begin + serial_number_length;


}
}