#include "can/core/message_handlers/presence_sensing.hpp"
#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "presence-sensing/core/presence_sensing_driver.hpp"
#include "presence-sensing/core/tasks/presence_sensing_driver_task.hpp"
#include "presence-sensing/tests/mock_presence_sensing.hpp"
#include "presence-sensing/tests/mock_presence_sensing_client.hpp"

using namespace presence_sensing_message_handler;
using namespace can_messages;

SCENARIO(
    "Presence sensing handler gets ReadPresenceSensingVoltageRequest msgs") {
    GIVEN("a presence sensing client queue and a presence sensing handler") {
        test_mocks::MockHeadQueueClient ps_client;
        auto handler = PresenceSensingHandler(ps_client);

        std::variant<std::monostate, ReadPresenceSensingVoltageRequest> tmp;
        WHEN(
            "presence sensing handler gets called with "
            "ReadPresenceSensingVoltageRequest") {
            THEN('presence sensing requests get enqued in ps_client queue') {
                handler.handle(decltype(handler)::MessageType(
                    ReadPresenceSensingVoltageRequest()));
                handler.handle(decltype(handler)::MessageType(
                    ReadPresenceSensingVoltageRequest()));
                handler.handle(decltype(handler)::MessageType(
                    ReadPresenceSensingVoltageRequest()));
                REQUIRE(ps_client.messages.size() == 3);
            }
        }
    }
}