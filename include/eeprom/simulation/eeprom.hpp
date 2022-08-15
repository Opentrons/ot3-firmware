#pragma once

#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iterator>
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
                LOG("Writing %d bytes to address %X", data_size,
                    current_address);
                backing.write(iter, current_address, data_size);
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
        backing.read(data, current_address, size);
        current_address += size;
        return true;
    }

    void set_write_protect(bool enabled) final {
        LOG("Setting write protect enabled to '%s'",
            enabled ? "enabled" : "disabled");
        write_protected = enabled;
    }

    struct BackingStore {
        static constexpr std::string_view TEMPFILE_KEY = "<temp file>";
        static constexpr size_t BACKING_SIZE =
            static_cast<size_t>(hardware_iface::EEpromMemorySize::ST_16_KBYTE);
        BackingStore(const BackingStore&) = delete;
        auto operator=(const BackingStore&) -> BackingStore& = delete;
        BackingStore(BackingStore&&) = delete;
        auto operator=(BackingStore&&) -> BackingStore&& = delete;
        explicit BackingStore(const po::variables_map& variables)
            : backing(get_prepped_backing_file(variables)) {}
        ~BackingStore() {
            if (backing) {
                if (!std::ferror(backing)) {
                    std::fflush(backing);
                }
                std::fclose(backing);
            }
        }
        static auto get_temp_file() -> std::FILE* {
            auto tempdir = std::filesystem::temp_directory_path();
            auto temp_path = tempdir / "eeprom.bin";
            LOG("Backing up eeprom with tempfile at %s", temp_path.c_str());
            return std::fopen(temp_path.c_str(), "w+b");
        }
        static auto get_backing_file(std::string pathstr) -> std::FILE* {
            auto path = std::filesystem::path(pathstr);
            path.make_preferred();
            LOG("Backing up eeprom with %s", path.c_str());
            return std::fopen(path.c_str(), "a+b");
        }
        static auto get_prepped_backing_file(std::string pathstr)
            -> std::FILE* {
            auto file = ((pathstr == TEMPFILE_KEY) ? get_temp_file()
                                                   : get_backing_file(pathstr));
            if (!file) {
                fprintf(stderr, "Could not open backing file %s: %d %s\n",
                        pathstr.c_str(), errno, strerror(errno));
                std::abort();
            }
            // what's the current end of the file? (we may open with append)
            auto start = std::ftell(file);
            std::fseek(file, 0, SEEK_END);
            auto size = static_cast<size_t>(std::ftell(file));
            // extend file if necessary but don't truncate
            if (size < BACKING_SIZE) {
                std::fseek(file, BACKING_SIZE, SEEK_SET);
                std::fseek(file, start, SEEK_SET);
                auto tmp_backing = std::array<char, BACKING_SIZE>{};
                tmp_backing.fill(0xff);
                std::fwrite(tmp_backing.data(), sizeof(tmp_backing[0]),
                            BACKING_SIZE - start, file);
                std::fflush(file);
                LOG("Extended backing file to %uB by writing %uB from %ld",
                    BACKING_SIZE, BACKING_SIZE - start, start);
            }
            // pointer back to 0, though we'll be doing a lot of SEEK_SET so it
            // doesn't really matter
            std::fseek(file, 0, SEEK_SET);
            LOG("got file at %p\n", file);
            return file;
        }
        static auto get_prepped_backing_file(const po::variables_map& variables)
            -> FILE* {
            return get_prepped_backing_file(
                variables["eeprom-filename"].as<std::string>());
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
        auto read(uint8_t* readbuf, uint16_t address, size_t size) -> void {
            if (std::fseek(backing, address, SEEK_SET)) {
                fprintf(stderr,
                        "fseek to %d for EEPROM read on file %p failed: eof=%d "
                        "err=%d %d %s",
                        address, backing, std::feof(backing),
                        std::ferror(backing), errno, strerror(errno));
            }
            auto read_count =
                std::fread(readbuf, sizeof(readbuf[0]), size, backing);
            if (read_count != size) {
                if (std::feof(backing)) {
                    fprintf(stderr,
                            "EEPROM read of %ld bytes from %d read only %ld "
                            "and hit eof\n",
                            size, address, read_count);
                    std::abort();
                } else if (std::ferror(backing)) {
                    fprintf(stderr,
                            "EEPROM read of %ld bytes from %d on file %p read "
                            "only %ld hit error: %d %s\n",
                            size, address, backing, read_count, errno,
                            strerror(errno));
                    std::abort();
                }
            }
        }

        auto write(const uint8_t* writebuf, uint16_t address, size_t size)
            -> void {
            if (std::fseek(backing, address, SEEK_SET)) {
                fprintf(stderr,
                        "fseek to %d on file %p for EEPROM write failed: "
                        "eof=%d err=%d %d %s",
                        address, backing, std::feof(backing),
                        std::ferror(backing), errno, strerror(errno));
            }

            auto write_count =
                std::fwrite(writebuf, sizeof(writebuf[0]), size, backing);
            if (write_count != size) {
                if (std::feof(backing)) {
                    fprintf(stderr, "EEPROM write of %ld bytes to %d hit eof\n",
                            size, address);
                    std::abort();
                } else if (std::ferror(backing)) {
                    fprintf(stderr,
                            "EEPROM write of %ld bytes to %d on file %p caused "
                            "error: %d %s\n",
                            size, address, backing, errno, strerror(errno));
                    std::abort();
                }
            }
            std::fflush(backing);
        }

      private:
        std::FILE* backing;
    };

  private:
    BackingStore backing;
    types::address current_address{0};
    bool write_protected{true};
};

}  // namespace simulator
}  // namespace eeprom
