#include <cstring>

#include "bootloader/core/ids.h"
#include "bootloader/core/message_handler.h"
#include "bootloader/core/node_id.h"
#include "bootloader/core/updater.h"
#include "catch2/catch.hpp"
#include "common/core/app_update.h"
#include "common/core/version.h"

/** Stub out get node id. */
CANNodeId get_node_id(void) { return can_nodeid_pipette_left_bootloader; }

version version_stub = {
    .version = 0xdeadbeef, .flags = 0xc0ffeebb, .sha = "abcdef0"};

extern "C" const version* version_get() { return &version_stub; }

/** Stub fw update functions.*/

FwUpdateReturn fw_update_initialize(UpdateState*) { return fw_update_ok; }

FwUpdateReturn fw_update_data(UpdateState*, uint32_t, const uint8_t*, uint8_t) {
    return fw_update_ok;
}

FwUpdateReturn fw_update_complete(UpdateState*, uint32_t, uint32_t) {
    return fw_update_invalid_size;
}

void fw_update_start_application() {}

FwUpdateReturn fw_update_erase_application(UpdateState*) {
    return fw_update_ok;
}

/** Stub the update flags. */
update_flag_type app_update_flags() { return 0xABE0DEAD; }

SCENARIO("device info") {
    GIVEN("a request") {
        Message request;
        request.arbitration_id.parts.function_code = 0;
        request.arbitration_id.parts.node_id = get_node_id();
        request.arbitration_id.parts.originating_node_id = can_nodeid_host;
        request.arbitration_id.parts.message_id =
            can_messageid_device_info_request;
        request.arbitration_id.parts.padding = 0;
        request.size = sizeof(uint32_t);
        request.data[0] = 0xD3;
        request.data[1] = 0x4D;
        request.data[2] = 0xB3;
        request.data[3] = 0x3F;

        WHEN("received") {
            Message response;
            auto error = handle_message(&request, &response);
            THEN("it returns response") {
                REQUIRE(error == handle_message_has_response);
            }
            THEN("it populates response arbitration id") {
                REQUIRE(response.arbitration_id.parts.function_code == 0);
                REQUIRE(response.arbitration_id.parts.node_id ==
                        can_nodeid_host);
                REQUIRE(response.arbitration_id.parts.originating_node_id ==
                        get_node_id());
                REQUIRE(response.arbitration_id.parts.message_id ==
                        can_messageid_device_info_response);
            }
            THEN("it populates response body") {
                REQUIRE(response.size == 25);
                REQUIRE(response.data[0] == 0xD3);
                REQUIRE(response.data[1] == 0x4D);
                REQUIRE(response.data[2] == 0xB3);
                REQUIRE(response.data[3] == 0x3F);
                REQUIRE(response.data[4] == 0xDE);
                REQUIRE(response.data[5] == 0xAD);
                REQUIRE(response.data[6] == 0xBE);
                REQUIRE(response.data[7] == 0xEF);
                REQUIRE(response.data[8] == 0xC0);
                REQUIRE(response.data[9] == 0xFF);
                REQUIRE(response.data[10] == 0xEE);
                REQUIRE(response.data[11] == 0xBB);
                REQUIRE(response.data[12] == 'a');
                REQUIRE(response.data[13] == 'b');
                REQUIRE(response.data[14] == 'c');
                REQUIRE(response.data[15] == 'd');
                REQUIRE(response.data[16] == 'e');
                REQUIRE(response.data[17] == 'f');
                REQUIRE(response.data[18] == '0');
                REQUIRE(response.data[19] == 0);
                REQUIRE(response.data[20] == 'a');
                REQUIRE(response.data[21] == '1');
                REQUIRE(response.data[22] == 0);
                REQUIRE(response.data[23] == 0);
                REQUIRE(response.data[24] == 0);
            }
        }
    }
}

SCENARIO("update data bad checksum error") {
    GIVEN("a request") {
        Message request;
        request.arbitration_id.parts.function_code = 0;
        request.arbitration_id.parts.node_id = get_node_id();
        request.arbitration_id.parts.originating_node_id = can_nodeid_host;
        request.arbitration_id.parts.message_id = can_messageid_fw_update_data;
        request.arbitration_id.parts.padding = 0;
        std::memset(request.data, 0, sizeof(request.data));
        request.data[0] = 0xDE;
        request.data[1] = 0xAD;
        request.data[2] = 0xBE;
        request.data[3] = 0xEF;
        request.data[4] = 0xC0;
        request.data[5] = 0xFF;
        request.data[6] = 0xEE;
        request.data[7] = 0x00;

        request.size = 64;

        WHEN("received") {
            Message response;
            auto error = handle_message(&request, &response);
            THEN("it returns response") {
                REQUIRE(error == handle_message_has_response);
            }
            THEN("it populates response arbitration id") {
                REQUIRE(response.arbitration_id.parts.function_code == 0);
                REQUIRE(response.arbitration_id.parts.node_id ==
                        can_nodeid_host);
                REQUIRE(response.arbitration_id.parts.originating_node_id ==
                        get_node_id());
                REQUIRE(response.arbitration_id.parts.message_id ==
                        can_messageid_fw_update_data_ack);
            }
            THEN("it populates response body") {
                REQUIRE(response.size == 10);
                REQUIRE(response.data[0] == 0xDE);
                REQUIRE(response.data[1] == 0xAD);
                REQUIRE(response.data[2] == 0xBE);
                REQUIRE(response.data[3] == 0xEF);
                REQUIRE(response.data[4] == 0xC0);
                REQUIRE(response.data[5] == 0xFF);
                REQUIRE(response.data[6] == 0xEE);
                REQUIRE(response.data[7] == 0x00);
                REQUIRE(response.data[8] ==
                        ((can_errorcode_bad_checksum >> 8) & 0xFF));
                REQUIRE(response.data[9] ==
                        (can_errorcode_bad_checksum & 0xFF));
            }
        }
    }
}
SCENARIO("update data invalid size") {
    GIVEN("a request") {
        Message request;
        request.arbitration_id.parts.function_code = 0;
        request.arbitration_id.parts.node_id = get_node_id();
        request.arbitration_id.parts.originating_node_id = can_nodeid_host;
        request.arbitration_id.parts.message_id = can_messageid_fw_update_data;
        request.arbitration_id.parts.padding = 0;
        std::memset(request.data, 0, sizeof(request.data));
        request.data[0] = 0xDE;
        request.data[1] = 0xAD;
        request.data[2] = 0xBE;
        request.data[3] = 0xEF;
        request.data[4] = 0xC0;
        request.data[5] = 0xFF;
        request.data[6] = 0xEE;
        request.data[7] = 0xBB;

        request.size = 8;

        WHEN("received") {
            Message response;
            auto error = handle_message(&request, &response);
            THEN("it returns response") {
                REQUIRE(error == handle_message_has_response);
            }
            THEN("it populates response arbitration id") {
                REQUIRE(response.arbitration_id.parts.function_code == 0);
                REQUIRE(response.arbitration_id.parts.node_id ==
                        can_nodeid_host);
                REQUIRE(response.arbitration_id.parts.originating_node_id ==
                        get_node_id());
                REQUIRE(response.arbitration_id.parts.message_id ==
                        can_messageid_fw_update_data_ack);
            }
            THEN("it populates response body") {
                REQUIRE(response.size == 10);
                REQUIRE(response.data[0] == 0xDE);
                REQUIRE(response.data[1] == 0xAD);
                REQUIRE(response.data[2] == 0xBE);
                REQUIRE(response.data[3] == 0xEF);
                REQUIRE(response.data[8] ==
                        ((can_errorcode_invalid_size >> 8) & 0xFF));
                REQUIRE(response.data[9] ==
                        (can_errorcode_invalid_size & 0xFF));
            }
        }
    }
}

SCENARIO("update data complete success") {
    GIVEN("a request") {
        Message request;
        request.arbitration_id.parts.function_code = 0;
        request.arbitration_id.parts.node_id = get_node_id();
        request.arbitration_id.parts.originating_node_id = can_nodeid_host;
        request.arbitration_id.parts.message_id =
            can_messageid_fw_update_complete;
        request.arbitration_id.parts.padding = 0;
        request.data[0] = 0xDE;
        request.data[1] = 0xAD;
        request.data[2] = 0xBE;
        request.data[3] = 0xEF;
        request.data[4] = 0xC0;
        request.data[5] = 0xFF;
        request.data[6] = 0xEE;
        request.data[7] = 0xBB;
        request.size = 12;

        WHEN("received") {
            Message response;
            auto error = handle_message(&request, &response);
            THEN("it returns response") {
                REQUIRE(error == handle_message_has_response);
            }
            THEN("it populates response arbitration id") {
                REQUIRE(response.arbitration_id.parts.function_code == 0);
                REQUIRE(response.arbitration_id.parts.node_id ==
                        can_nodeid_host);
                REQUIRE(response.arbitration_id.parts.originating_node_id ==
                        get_node_id());
                REQUIRE(response.arbitration_id.parts.message_id ==
                        can_messageid_fw_update_complete_ack);
            }
            THEN("it populates response body") {
                REQUIRE(response.size == 6);
                REQUIRE(response.data[0] == 0xDE);
                REQUIRE(response.data[1] == 0xAD);
                REQUIRE(response.data[2] == 0xBE);
                REQUIRE(response.data[3] == 0xEF);
                REQUIRE(response.data[4] ==
                        ((can_errorcode_invalid_size >> 8) & 0xFF));
                REQUIRE(response.data[5] ==
                        (can_errorcode_invalid_size & 0xFF));
            }
        }
    }
}

SCENARIO("update data complete error") {
    GIVEN("a request") {
        Message request;
        request.arbitration_id.parts.function_code = 0;
        request.arbitration_id.parts.node_id = get_node_id();
        request.arbitration_id.parts.originating_node_id = can_nodeid_host;
        request.arbitration_id.parts.message_id =
            can_messageid_fw_update_complete;
        request.arbitration_id.parts.padding = 0;
        request.data[0] = 0xDE;
        request.data[1] = 0xAD;
        request.data[2] = 0xBE;
        request.data[3] = 0xEF;
        request.data[4] = 0xC0;
        request.data[5] = 0xFF;
        request.data[6] = 0xEE;
        request.data[7] = 0xBB;
        request.size = 8;

        WHEN("received") {
            Message response;
            auto error = handle_message(&request, &response);
            THEN("it returns response") {
                REQUIRE(error == handle_message_has_response);
            }
            THEN("it populates response arbitration id") {
                REQUIRE(response.arbitration_id.parts.function_code == 0);
                REQUIRE(response.arbitration_id.parts.node_id ==
                        can_nodeid_host);
                REQUIRE(response.arbitration_id.parts.originating_node_id ==
                        get_node_id());
                REQUIRE(response.arbitration_id.parts.message_id ==
                        can_messageid_fw_update_complete_ack);
            }
            THEN("it populates response body") {
                REQUIRE(response.size == 6);

                REQUIRE(response.data[0] == 0xDE);
                REQUIRE(response.data[1] == 0xAD);
                REQUIRE(response.data[2] == 0xBE);
                REQUIRE(response.data[3] == 0xEF);
                REQUIRE(response.data[4] ==
                        ((can_errorcode_invalid_size >> 8) & 0xFF));
                REQUIRE(response.data[5] ==
                        (can_errorcode_invalid_size & 0xFF));
            }
        }
    }
}

SCENARIO("update get status") {
    GIVEN("a request") {
        Message request;
        request.arbitration_id.parts.function_code = 0;
        request.arbitration_id.parts.node_id = get_node_id();
        request.arbitration_id.parts.originating_node_id = can_nodeid_host;
        request.arbitration_id.parts.message_id =
            can_messageid_fw_update_status_request;
        request.arbitration_id.parts.padding = 0;
        request.size = 4;
        request.data[0] = 0xDE;
        request.data[1] = 0xAD;
        request.data[2] = 0xBE;
        request.data[3] = 0xEF;

        WHEN("received") {
            Message response;
            auto error = handle_message(&request, &response);
            THEN("it returns response") {
                REQUIRE(error == handle_message_has_response);
            }
            THEN("it populates response arbitration id") {
                REQUIRE(response.arbitration_id.parts.function_code == 0);
                REQUIRE(response.arbitration_id.parts.node_id ==
                        can_nodeid_host);
                REQUIRE(response.arbitration_id.parts.originating_node_id ==
                        get_node_id());
                REQUIRE(response.arbitration_id.parts.message_id ==
                        can_messageid_fw_update_status_response);
            }
            THEN("it populates response body") {
                REQUIRE(response.size == 8);
                REQUIRE(response.data[0] == 0xDE);
                REQUIRE(response.data[1] == 0xAD);
                REQUIRE(response.data[2] == 0xBE);
                REQUIRE(response.data[3] == 0xEF);
                REQUIRE(response.data[4] == 0xAB);
                REQUIRE(response.data[5] == 0xE0);
                REQUIRE(response.data[6] == 0xDE);
                REQUIRE(response.data[7] == 0xAD);
            }
        }
    }
}
