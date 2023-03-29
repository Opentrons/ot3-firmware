#include <cstring>

#include "catch2/catch.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/version.h"
#include "rear-panel/core/bin_msg_ids.hpp"
#include "rear-panel/core/messages.hpp"

using namespace rearpanel::messages;

SCENARIO("message deserializing works") {
    GIVEN("a get device info request body") {
        auto arr = std::array<uint8_t, 4>{0x00, 0x03, 0x00, 0x00};
        WHEN("constructed") {
            auto r = DeviceInfoRequest::parse(arr.begin(), arr.end());
            REQUIRE(r.index() != 0);
            auto msg_ptr = std::get_if<DeviceInfoRequest>(&r);
            REQUIRE(msg_ptr != 0);
            auto msg = *msg_ptr;
            THEN("it is converted to a the correct structure") {
                REQUIRE(msg.length == 0x0000);
            }
            THEN("it can be compared for equality") {
                auto other = msg;
                REQUIRE(other == msg);
                other.length = 10;
                REQUIRE(other != msg);
            }
        }
    }
}

SCENARIO("message serializing works") {
    GIVEN("a device info response message") {
        auto message =
            DeviceInfoResponse{.length = DeviceInfoResponse::get_length(),
                               .version = 0x00220033,
                               .flags = 0x11445566,
                               .shortsha{"abcdef0"},
                               .primary_revision = 'A',
                               .secondary_revision = 'B',
                               .tertiary_revision{"C"}};
        auto arr = std::array<uint8_t, 30>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized into a buffer too small for its values") {
            auto next_free = message.serialize(arr.begin(), arr.begin() + 14);
            THEN("it is written into the buffer to its end") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x04);
                REQUIRE(body.data()[2] == 0x00);
                REQUIRE(body.data()[3] == 0x14);
                REQUIRE(body.data()[4] == 0x00);
                REQUIRE(body.data()[5] == 0x22);
                REQUIRE(body.data()[6] == 0x00);
                REQUIRE(body.data()[7] == 0x33);
                REQUIRE(body.data()[8] == 0x11);
                REQUIRE(body.data()[9] == 0x44);
                REQUIRE(body.data()[10] == 0x55);
                REQUIRE(body.data()[11] == 0x66);
                REQUIRE(body.data()[12] == 'a');
                REQUIRE(body.data()[13] == 'b');
            }
            THEN("it does not write past the end of the buffer") {
                for (uint32_t i = 14; i < arr.size(); i++) {
                    REQUIRE(body.data()[i] == 0);
                }
            }
            THEN("size must be returned") {
                REQUIRE(next_free == arr.begin() + 14);
            }
        }
        WHEN("serialized into a buffer larger than needed") {
            auto next_free = message.serialize(arr.begin(), arr.end());
            THEN("it is fully written into the buffer") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x04);
                REQUIRE(body.data()[2] == 0x00);
                REQUIRE(body.data()[3] == 0x14);
                REQUIRE(body.data()[4] == 0x00);
                REQUIRE(body.data()[5] == 0x22);
                REQUIRE(body.data()[6] == 0x00);
                REQUIRE(body.data()[7] == 0x33);
                REQUIRE(body.data()[8] == 0x11);
                REQUIRE(body.data()[9] == 0x44);
                REQUIRE(body.data()[10] == 0x55);
                REQUIRE(body.data()[11] == 0x66);
                REQUIRE(body.data()[12] == 'a');
                REQUIRE(body.data()[13] == 'b');
                REQUIRE(body.data()[14] == 'c');
                REQUIRE(body.data()[15] == 'd');
                REQUIRE(body.data()[16] == 'e');
                REQUIRE(body.data()[17] == 'f');
                REQUIRE(body.data()[18] == '0');
                REQUIRE(body.data()[19] == 0);
                REQUIRE(body.data()[20] == 'A');
                REQUIRE(body.data()[21] == 'B');
                REQUIRE(body.data()[22] == 'C');
                REQUIRE(body.data()[23] == 0);
            }
            THEN("it does not write past the end of the buffer") {
                REQUIRE(body.data()[24] == 0);
            }
            THEN("size must be returned") {
                REQUIRE(next_free == arr.begin() + 25);
            }
        }
    }
}

SCENARIO("message parsing") {
    auto rear_panel_parser = rearpanel::messages::Parser{};
    GIVEN("a valid message id body") {
        auto arr = std::array<uint8_t, 4>{0x00, 0x03, 0x00, 0x00};
        WHEN("constructed") {
            auto message = rear_panel_parser.parse(
                rearpanel::ids::BinaryMessageId(0x0003), arr.begin(),
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                arr.end());
            THEN("monostate is not returned") { REQUIRE(message.index() != 0); }
            THEN("DeviceInfoRequest is returned") {
                auto typed =
                    std::get_if<rearpanel::messages::DeviceInfoRequest>(
                        &message);
                REQUIRE(typed != 0);
            }
        }
    }
    GIVEN("a invalid message length") {
        auto arr = std::array<uint8_t, 4>{0x00, 0x03, 0x00, 0x0A};
        WHEN("constructed") {
            auto message = rear_panel_parser.parse(
                rearpanel::ids::BinaryMessageId(0x0003), arr.begin(),
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                arr.end());
            THEN("monostate is returned") {
                REQUIRE(message.index() == 0);
                auto typed = std::get_if<std::monostate>(&message);
                REQUIRE(typed != 0);
            }
        }
    }
    GIVEN("a invalid message id body") {
        auto arr = std::array<uint8_t, 4>{0xAB, 0xCD, 0x00, 0x00};
        WHEN("constructed") {
            auto message = rear_panel_parser.parse(
                rearpanel::ids::BinaryMessageId(0xABCD), arr.begin(),
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                arr.end());
            THEN("monostate is returned") {
                REQUIRE(message.index() == 0);
                auto typed = std::get_if<std::monostate>(&message);
                REQUIRE(typed != 0);
            }
        }
    }
}

TEST_CASE("AddLightActionRequest parsing") {
    auto rear_panel_parser = rearpanel::messages::Parser{};
    using namespace rearpanel;
    GIVEN("valid input") {
        auto input = std::array<uint8_t, 11>{// Header
                                             0x04, 0x00, 0x00, 0x07,
                                             // Transition time
                                             0x00, 100,
                                             // Transition type
                                             0x01,
                                             // Colors RGBW
                                             0x10, 0x20, 0x30, 0x40};
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0400),
                                                   input.begin(), input.end());
            THEN("the message is correctly parsed") {
                REQUIRE(std::holds_alternative<messages::AddLightActionRequest>(
                    message));
                auto request =
                    std::get<messages::AddLightActionRequest>(message);
                REQUIRE(request.length == 7);
                REQUIRE(request.transition_time_ms == 100);
                REQUIRE(request.transition ==
                        ids::LightTransitionType::sinusoid);
                REQUIRE(request.red == 0x10);
                REQUIRE(request.green == 0x20);
                REQUIRE(request.blue == 0x30);
                REQUIRE(request.white == 0x40);
            }
        }
    }
    GIVEN("invalid input") {
        auto input = std::array<uint8_t, 11>{// Header
                                             0x04, 0x00, 0x00, 0x10,
                                             // Transition time
                                             0x00, 100,
                                             // Transition type
                                             0x01,
                                             // Colors RGBW
                                             0x10, 0x20, 0x30, 0x40};
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0400),
                                                   input.begin(), input.end());
            THEN("the message is not parsed") {
                REQUIRE(std::holds_alternative<std::monostate>(message));
            }
        }
    }
}

TEST_CASE("ClearLightActionStagingQueueRequest parsing") {
    auto rear_panel_parser = rearpanel::messages::Parser{};
    using namespace rearpanel;
    GIVEN("valid input") {
        auto input = std::array<uint8_t, 4>{
            // Header
            0x04,
            0x01,
            0x00,
            0x00,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0401),
                                                   input.begin(), input.end());
            THEN("the message is correctly parsed") {
                REQUIRE(std::holds_alternative<
                        messages::ClearLightActionStagingQueueRequest>(
                    message));
            }
        }
    }
    GIVEN("invalid input") {
        auto input = std::array<uint8_t, 4>{
            // Header w/ wrong length
            0x04,
            0x01,
            0x00,
            0x10,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0401),
                                                   input.begin(), input.end());
            THEN("the message is not parsed") {
                REQUIRE(std::holds_alternative<std::monostate>(message));
            }
        }
    }
}

TEST_CASE("StartLightActionRequest parsing") {
    auto rear_panel_parser = rearpanel::messages::Parser{};
    using namespace rearpanel;
    GIVEN("valid input") {
        auto input = std::array<uint8_t, 5>{
            // Header
            0x04,
            0x02,
            0x00,
            0x01,
            // Animation type looping
            0x00,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0402),
                                                   input.begin(), input.end());
            THEN("the message is correctly parsed") {
                REQUIRE(
                    std::holds_alternative<messages::StartLightActionRequest>(
                        message));
                auto request =
                    std::get<messages::StartLightActionRequest>(message);
                REQUIRE(request.animation == ids::LightAnimationType::looping);
            }
        }
    }
    GIVEN("invalid input") {
        auto input = std::array<uint8_t, 4>{
            // Header w/ wrong length
            0x04,
            0x02,
            0x00,
            0x10,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0402),
                                                   input.begin(), input.end());
            THEN("the message is not parsed") {
                REQUIRE(std::holds_alternative<std::monostate>(message));
            }
        }
    }
}

TEST_CASE("SetDeckLightRequest parsing") {
    auto rear_panel_parser = rearpanel::messages::Parser{};
    using namespace rearpanel;
    GIVEN("valid input") {
        auto input = std::array<uint8_t, 5>{
            // Header
            0x04,
            0x10,
            0x00,
            0x01,
            // On
            0x01,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0410),
                                                   input.begin(), input.end());
            THEN("the message is correctly parsed") {
                REQUIRE(std::holds_alternative<messages::SetDeckLightRequest>(
                    message));
                auto request = std::get<messages::SetDeckLightRequest>(message);
                REQUIRE(request.setting == 1);
            }
        }
    }
    GIVEN("invalid input") {
        auto input = std::array<uint8_t, 4>{
            // Header w/ wrong length
            0x04,
            0x10,
            0x00,
            0x10,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0410),
                                                   input.begin(), input.end());
            THEN("the message is not parsed") {
                REQUIRE(std::holds_alternative<std::monostate>(message));
            }
        }
    }
}

TEST_CASE("GetDeckLightRequest parsing") {
    auto rear_panel_parser = rearpanel::messages::Parser{};
    using namespace rearpanel;
    GIVEN("valid input") {
        auto input = std::array<uint8_t, 5>{
            // Header
            0x04,
            0x11,
            0x00,
            0x00,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0411),
                                                   input.begin(), input.end());
            THEN("the message is correctly parsed") {
                REQUIRE(std::holds_alternative<messages::GetDeckLightRequest>(
                    message));
            }
        }
    }
    GIVEN("invalid input") {
        auto input = std::array<uint8_t, 4>{
            // Header w/ wrong length
            0x04,
            0x11,
            0x00,
            0x10,
        };
        WHEN("parsed") {
            auto message = rear_panel_parser.parse(ids::BinaryMessageId(0x0411),
                                                   input.begin(), input.end());
            THEN("the message is not parsed") {
                REQUIRE(std::holds_alternative<std::monostate>(message));
            }
        }
    }
}

TEST_CASE("GetDeckLightResponse serializing") {
    using namespace rearpanel;
    auto input = messages::GetDeckLightResponse{.setting = 1};
    auto buffer = std::array<uint8_t, 10>();
    for (auto& b : buffer) {
        b = 0xFF;
    }
    GIVEN("large enough buffer") {
        THEN("serializing passes") {
            auto ret = input.serialize(buffer.begin(), buffer.end());
            REQUIRE(ret != buffer.end());
            REQUIRE(buffer[0] == 0x04);
            REQUIRE(buffer[1] == 0x12);
            REQUIRE(buffer[2] == 0x00);
            REQUIRE(buffer[3] == 0x01);
            REQUIRE(buffer[4] == 0x01);
        }
    }
    GIVEN("buffer too small") {
        THEN("serializing doesn't overrun the buffer") {
            auto ret = input.serialize(buffer.begin(), buffer.begin() + 2);
            REQUIRE(ret == buffer.begin() + 2);
            REQUIRE(buffer[0] == 0x04);
            REQUIRE(buffer[1] == 0x12);
            REQUIRE(buffer[2] == 0xFF);
            REQUIRE(buffer[3] == 0xFF);
            REQUIRE(buffer[4] == 0xFF);
        }
    }
}
