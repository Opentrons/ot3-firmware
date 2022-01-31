#include "bootloader/core/message_handler.h"
#include "bootloader/core/ids.h"
#include "bootloader/core/node_id.h"
#include "bootloader/core/version.h"
#include "catch2/catch.hpp"


NodeId get_node_id(void) {
    return can_nodeid_pipette_bootloader;
}

uint32_t get_version(void) {
    return 0xDEADBEEF;
}


SCENARIO("device info") {
    GIVEN("a request") {
        Message request;
        request.arbitration_id.parts.function_code=0;
        request.arbitration_id.parts.node_id=get_node_id();
        request.arbitration_id.parts.originating_node_id=can_nodeid_host;
        request.arbitration_id.parts.message_id=can_messageid_device_info_request;
        request.arbitration_id.parts.padding=0;

        WHEN("received") {
            Message response;
            auto error = handle_message(&request, &response);
            THEN("it returns ok") { REQUIRE(error == handle_message_ok); }
            THEN("it populates response arbitration id") {
                REQUIRE(response.arbitration_id.parts.function_code == 0);
                REQUIRE(response.arbitration_id.parts.node_id == can_nodeid_host);
                REQUIRE(response.arbitration_id.parts.originating_node_id == get_node_id());
                REQUIRE(response.arbitration_id.parts.message_id == can_messageid_device_info_response);
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