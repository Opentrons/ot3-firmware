#include <cstring>

#include "bootloader/core/ids.h"
#include "bootloader/core/message_handler.h"
#include "bootloader/core/node_id.h"
#include "bootloader/core/updater.h"
#include "bootloader/core/version.h"
#include "catch2/catch.hpp"
#include "common/core/app_update.h"

/** Stub out get node id. */
CANNodeId get_node_id(void) { return can_nodeid_pipette_left_bootloader; }

/** Stub out get version. */
uint32_t get_version(void) { return 0xDEADBEEF; }

/** Stub fw update functions.*/

FwUpdateReturn fw_update_initialize(UpdateState* state) { return fw_update_ok; }

FwUpdateReturn fw_update_data(UpdateState* state, uint32_t address,
                              const uint8_t* data, uint8_t length) {
    return fw_update_ok;
}

FwUpdateReturn fw_update_complete(UpdateState* state, uint32_t num_messages,
                                  uint32_t crc32) {
    return fw_update_invalid_size;
}

void fw_update_start_application() {}

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
                REQUIRE(response.size == 4);
                REQUIRE(response.data[0] == 0xDE);
                REQUIRE(response.data[1] == 0xAD);
                REQUIRE(response.data[2] == 0xBE);
                REQUIRE(response.data[3] == 0xEF);
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
                REQUIRE(response.size == 6);
                REQUIRE(response.data[0] == 0xDE);
                REQUIRE(response.data[1] == 0xAD);
                REQUIRE(response.data[2] == 0xBE);
                REQUIRE(response.data[3] == 0xEF);
                REQUIRE(response.data[4] ==
                        ((can_errorcode_bad_checksum >> 8) & 0xFF));
                REQUIRE(response.data[5] ==
                        (can_errorcode_bad_checksum & 0xFF));
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
        request.size = 4;

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
                REQUIRE(response.size == 2);
                REQUIRE(response.data[0] ==
                        ((can_errorcode_invalid_size >> 8) & 0xFF));
                REQUIRE(response.data[1] ==
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
        request.size = 0;

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
                REQUIRE(response.size == 4);
                REQUIRE(response.data[0] == 0xAB);
                REQUIRE(response.data[1] == 0xE0);
                REQUIRE(response.data[2] == 0xDE);
                REQUIRE(response.data[3] == 0xAD);
            }
        }
    }
}
