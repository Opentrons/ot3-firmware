#pragma once

#include <array>
#include <functional>
#include <string>

#include "boost/program_options.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/types.hpp"
#include "i2c/simulation/device.hpp"

namespace eeprom {
namespace simulator {

using namespace i2c::hardware;
namespace po = boost::program_options;

class EEProm : public I2CDeviceBase,
               public hardware_iface::EEPromHardwareIface {
  public:
    static auto add_options(po::options_description& cmdline_desc,
                            po::options_description& env_desc)
        -> std::function<std::string(std::string)> {
        return BackingStore::add_options(cmdline_desc, env_desc);
    }
    explicit EEProm(po::variables_map& options)
        : I2CDeviceBase(types::DEVICE_ADDRESS), backing(options) {}
    EEProm(hardware_iface::EEPromChipType chip, po::variables_map& options)
        : I2CDeviceBase(types::DEVICE_ADDRESS),
          hardware_iface::EEPromHardwareIface(chip),
          backing(options) {}

    auto handle_write(const uint8_t* data, uint16_t size) -> bool {
        auto* iter = data;

        // Read the address
        iter = bit_utils::bytes_to_int(
            iter, data + this->get_eeprom_addr_bytes(), current_address);
        // if the address is configued as 8 bit we need to shift the
        // current_address toward the LSB
        if (this->get_eeprom_addr_bytes() ==
            static_cast<size_t>(
                hardware_iface::EEPromAddressType::EEPROM_ADDR_8_BIT)) {
            current_address = current_address >> 8;
        }
        auto data_size = size - this->get_eeprom_addr_bytes();
        if (data_size > 0) {
            if (!write_protected) {
                // Let the exception happen. Catch errors!
                std::copy_n(iter, data_size,
                            backing.value().begin() + current_address);
                LOG("Writing %d bytes to address %X", data_size,
                    current_address);
            } else {
                LOG("Write protect is enabled. Cannot write.");
            }
        } else {
            LOG("Updating address to %X", current_address);
        }

        return true;
    }

    auto handle_read(uint8_t* data, uint16_t size) -> bool {
        LOG("Reading %d bytes from address %X", size, current_address);
        for (auto i = 0; i < size; i++) {
            data[i] = backing.value()[current_address++];
        }
        return true;
    }

    void set_write_protect(bool enabled) final {
        LOG("Setting write protect enabled to '%s'",
            enabled ? "enabled" : "disabled");
        write_protected = enabled;
    }

    struct BackingStore {
        static constexpr std::string_view TEMPFILE_KEY = "<temp file>";
        using BackingType =
            std::array<uint8_t,
                       static_cast<size_t>(
                           hardware_iface::EEpromMemorySize::ST_16_KBYTE)>;
        explicit BackingStore(const po::variables_map& variables) {
            auto filename = variables["eeprom-filename"].as<std::string>();
            LOG("Using eeprom backing store %s", filename.c_str());
            backing.fill(0xff);
        }
        static auto add_options(po::options_description& cmdline,
                                po::options_description& env)
            -> std::function<std::string(std::string)> {
            cmdline.add_options()(
                "eeprom-filename,f",
                po::value<std::string>()->default_value(
                    std::string(TEMPFILE_KEY)),
                "path to backing file for eeprom. if unspecified, a temp will "
                "be used. May be specified in an environment file called "
                "EEPROM_FILENAME.");
            env.add_options()("EEPROM_FILENAME",
                              po::value<std::string>()->default_value(
                                  std::string(TEMPFILE_KEY)));
            return [](std::string input_val) -> std::string {
                if (input_val == "EEPROM_FILENAME") {
                    return "eeprom-filename";
                }
                return "";
            };
        }
        auto value() -> BackingType& { return backing; }

      private:
        BackingType backing{};
    };

  private:
    BackingStore backing;
    types::address current_address{0};
    bool write_protected{true};
};

}  // namespace simulator
}  // namespace eeprom
