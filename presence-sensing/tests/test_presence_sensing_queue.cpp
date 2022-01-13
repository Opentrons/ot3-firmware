#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "can/core/messages.hpp"
#include "can/core/message_handlers/presence_sensing.hpp"
#include "presence-sensing/core/presence_sensing_driver.hpp"
#include "presence-sensing/core/tasks/presence_sensing_driver_task.hpp"
#include "presence-sensing/tests/mock_presence_sensing.hpp"
#include "presence-sensing/tests/mock_presence_sensing_client.hpp"

using namespace presence_sensing;

SCENARIO("enqueue presence sensing request msgs") {
    GIVEN("a presence sensing message handler") {
        test_mocks::MockMessageQueue<ReadPresenceSensingVoltageRequest> queue;
        ReadPresenceSensingVoltageRequest m;
        test_mocks::MockPresenceSensingClient ps_client;
        auto handler = PresenceSensingHandler(ps_client);

        WHEN("presence sensing handler called") {
            THEN('presence sensing requests get enqued in ps_client queue') {
                queue.try_write(m);
                queue.try_write(m);
                queue.try_write(m);
                while (!queue.empty())
                {
                    handler.handle(queue.back());
                    queue.pop_back();
                }


                REQUIRE(ps_client.get_size() == 3)
            }
        }
    }


}