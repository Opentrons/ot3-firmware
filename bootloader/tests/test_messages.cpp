#include "bootloader/core/messages.h"
#include "bootloader/core/util.h"
#include "catch2/catch.hpp"

SCENARIO("empty message") {
    GIVEN("an empty payload message") {
        auto arr = std::array<uint8_t, 4>{// Message Index
                                          0xde, 0xad, 0xbe, 0xef};
        WHEN("parsed") {
            uint32_t message_index;
            auto error =
                parse_empty_message(arr.data(), arr.size(), &message_index);
            THEN("it returns ok") { REQUIRE(error == can_errorcode_ok); }
            THEN("its fields are populated") {
                REQUIRE(message_index == 0xdeadbeef);
            }
        }
    }
}

SCENARIO("empty message errors") {
    GIVEN("a null pointer") {
        WHEN("parsed") {
            uint32_t message_index;
            auto error = parse_empty_message(nullptr, 4, &message_index);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_size);
            }
        }
    }

    GIVEN("a null pointer for outparam") {
        auto arr = std::array<uint8_t, 4>{};
        WHEN("parsed") {
            auto error = parse_empty_message(arr.data(), arr.size(), nullptr);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_input);
            }
        }
    }

    GIVEN("a message with invalid size") {
        auto arr = std::array<uint8_t, 5>{};
        WHEN("parsed") {
            uint32_t message_index;
            auto error =
                parse_empty_message(arr.data(), arr.size(), &message_index);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_size);
            }
        }
    }
}

SCENARIO("update data") {
    GIVEN("a message with a data byte count of 0") {
        auto arr = std::array<uint8_t, 60>{
            // Message Index
            0xde, 0xad, 0xbe, 0xef,
            // Address
            0xa, 0xb, 0xc, 0xd,
            // Size
            0x0,
            // Reserved
            0x0,
            // Data
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0,
            // Checksum.
            0xfc, 0x9a};

        WHEN("parsed") {
            UpdateData result;
            auto error = parse_update_data(arr.data(), arr.size(), &result);
            THEN("it returns ok") { REQUIRE(error == can_errorcode_ok); }
            THEN("its fields are populated") {
                REQUIRE(result.message_index == 0xdeadbeef);
                REQUIRE(result.address == 0x0a0b0c0d);
                REQUIRE(result.num_bytes == 0);
                REQUIRE(result.reserved == 0);
                REQUIRE(result.data == arr.data() + 10);
                REQUIRE(result.checksum == 0xFC9A);
            }
        }
    }

    GIVEN("a message with full data byte count") {
        auto arr = std::array<uint8_t, 60>{
            // Message Index
            0xde, 0xad, 0xbe, 0xef,
            // Address
            0x89, 0xab, 0xcd, 0xef,
            // Size
            48,
            // Reserved
            0x0,
            // Data
            0, 1, 2, 3, 4, 5, 6, 7, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
            0x17, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x30, 0x31,
            0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x40, 0x41, 0x42, 0x43, 0x44,
            0x45, 0x46, 0x47, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
            // Checksum.
            0xF1, 0x80};

        WHEN("parsed") {
            UpdateData result;
            auto error = parse_update_data(arr.data(), arr.size(), &result);
            THEN("it returns ok") { REQUIRE(error == can_errorcode_ok); }
            THEN("its fields are populated") {
                REQUIRE(result.message_index == 0xdeadbeef);
                REQUIRE(result.address == 0x89abcdef);
                REQUIRE(result.num_bytes == 48);
                REQUIRE(result.reserved == 0);
                REQUIRE(result.data == arr.data() + 10);
                REQUIRE(result.checksum == 0xF180);
            }
        }
    }
}

SCENARIO("update data errors") {
    GIVEN("a null pointer") {
        WHEN("parsed") {
            UpdateData result;
            auto error = parse_update_data(nullptr, 64, &result);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_size);
            }
        }
    }

    GIVEN("a null pointer for outparam") {
        auto arr = std::array<uint8_t, 64>{};
        WHEN("parsed") {
            auto error = parse_update_data(arr.data(), arr.size(), nullptr);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_input);
            }
        }
    }

    GIVEN("a message with invalid size") {
        auto arr = std::array<uint8_t, 65>{};
        WHEN("parsed") {
            UpdateData result;
            auto error = parse_update_data(arr.data(), arr.size(), &result);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_size);
            }
        }
    }

    GIVEN("a message with invalid data byte count") {
        auto arr = std::array<uint8_t, 60>{
            // Message Index
            0xde, 0xad, 0xbe, 0xef,
            // Address
            0xa, 0xb, 0xc, 0xd,
            // Size
            52,
            // Reserved
            0x0,
            // Data
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0,
            // Checksum.
            0xFC, 0x9A};
        WHEN("parsed") {
            UpdateData result;
            auto error = parse_update_data(arr.data(), arr.size(), &result);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_byte_count);
            }
        }
    }

    GIVEN("a message with incorrect checksum") {
        auto arr = std::array<uint8_t, 60>{
            // Message Index
            0xde, 0xad, 0xbe, 0xef,
            // Address
            0x89, 0xab, 0xcd, 0xef,
            // Size
            48,
            // Reserved
            0x0,
            // Data
            0, 1, 2, 3, 4, 5, 6, 7, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
            0x17, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x30, 0x31,
            0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x40, 0x41, 0x42, 0x43, 0x44,
            0x45, 0x46, 0x47, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
            // Checksum.
            0xF1, 0x93};
        WHEN("parsed") {
            UpdateData result;
            auto error = parse_update_data(arr.data(), arr.size(), &result);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_bad_checksum);
            }
        }
    }
}

SCENARIO("update data complete") {
    GIVEN("a message") {
        auto arr = std::array<uint8_t, 12>{// Message Index
                                           0xde, 0xad, 0xbe, 0xef,
                                           // Count
                                           0xfe, 0xdc, 0xba, 0x98,
                                           // CRC32
                                           0xff, 0xfe, 0xfd, 0xfc};
        WHEN("parsed") {
            UpdateComplete result;
            auto error = parse_update_complete(arr.data(), arr.size(), &result);
            THEN("it returns ok") { REQUIRE(error == can_errorcode_ok); }
            THEN("its fields are populated") {
                REQUIRE(result.message_index == 0xdeadbeef);
                REQUIRE(result.num_messages == 0xfedcba98);
                REQUIRE(result.crc32 == 0xfffefdfc);
            }
        }
    }
}

SCENARIO("update data complete errors") {
    GIVEN("a null pointer") {
        WHEN("parsed") {
            UpdateComplete result;
            auto error = parse_update_complete(nullptr, 8, &result);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_size);
            }
        }
    }

    GIVEN("a null pointer for result") {
        auto arr = std::array<uint8_t, 12>{// Message Index
                                           0xde, 0xad, 0xbe, 0xef,
                                           // Count
                                           0xfe, 0xdc, 0xba, 0x98,
                                           // CRC32
                                           0xff, 0xfe, 0xfd, 0xfc};
        WHEN("parsed") {
            auto error = parse_update_complete(arr.data(), arr.size(), nullptr);
            THEN("it returns error") {
                REQUIRE(error == can_errorcode_invalid_input);
            }
        }
    }

    GIVEN("a message with incorrect size") {
        auto arr = std::array<uint8_t, 12>{// Message Index
                                           0xde, 0xad, 0xbe, 0xef,
                                           // Count
                                           0xfe, 0xdc, 0xba, 0x98,
                                           // CRC32
                                           0xff, 0xfe, 0xfd, 0xfc};
        WHEN("parsed") {
            UpdateComplete result;
            auto error =
                parse_update_complete(arr.data(), arr.size() + 1, &result);
            THEN("it returns ok") {
                REQUIRE(error == can_errorcode_invalid_size);
            }
        }
    }
}
