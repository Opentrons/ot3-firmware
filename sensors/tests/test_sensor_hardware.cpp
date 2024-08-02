#include <concepts>

#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/tests/mock_hardware.hpp"

auto version_wrapper = sensors::hardware::SensorHardwareVersionSingleton();
constexpr auto sensor_id_primary = can::ids::SensorId::S0;
constexpr auto sensor_id_secondary = can::ids::SensorId::S1;

SCENARIO("Multiple Sensors connected") {
    auto sync_control = sensors::hardware::SensorHardwareSyncControlSingleton();
    test_mocks::MockSensorHardware mock_hw =
        test_mocks::MockSensorHardware{version_wrapper, sync_control};
    GIVEN("One Sensor In use") {
        WHEN("Sensor not enabled") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == false);
        }
        WHEN("Sensor enabled") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync_enabled(sensor_id_primary, true);
            mock_hw.set_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
        }
        WHEN("Sensor required") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync_required(sensor_id_primary, true);
            mock_hw.set_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
        }
        WHEN("Sensor enabled and other sensor sets") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync_enabled(sensor_id_primary, true);
            mock_hw.set_sync(sensor_id_secondary);
            REQUIRE(mock_hw.get_sync_state_mock() == false);
        }
        WHEN("Resetting sync for in use sensor") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync_enabled(sensor_id_primary, true);
            mock_hw.set_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
            mock_hw.reset_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == false);
        }
        WHEN("Resetting sync for in other sensor") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync_enabled(sensor_id_primary, true);
            mock_hw.set_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
            mock_hw.reset_sync(sensor_id_secondary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
        }
    }
    GIVEN("Two sensors enabled") {
        mock_hw.set_sync_enabled(sensor_id_primary, true);
        mock_hw.set_sync_enabled(sensor_id_secondary, true);
        WHEN("primary sets") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
        }
        WHEN("Secondary sets") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync(sensor_id_secondary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
        }
        WHEN("both are set") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync(sensor_id_primary);
            mock_hw.set_sync(sensor_id_secondary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
        }
        WHEN("Sensors are disable") {
            mock_hw.set_sync(sensor_id_primary);
            mock_hw.set_sync(sensor_id_secondary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
            mock_hw.set_sync_enabled(sensor_id_primary, false);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
            mock_hw.set_sync_enabled(sensor_id_secondary, false);
            REQUIRE(mock_hw.get_sync_state_mock() == false);
        }
    }
    GIVEN("Two sensors required") {
        mock_hw.set_sync_required(sensor_id_primary, true);
        mock_hw.set_sync_required(sensor_id_secondary, true);
        WHEN("primary sets") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync(sensor_id_primary);
            REQUIRE(mock_hw.get_sync_state_mock() == false);
        }
        WHEN("Secondary sets") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync(sensor_id_secondary);
            REQUIRE(mock_hw.get_sync_state_mock() == false);
        }
        WHEN("both are set") {
            REQUIRE(mock_hw.get_sync_state_mock() == false);
            mock_hw.set_sync(sensor_id_primary);
            mock_hw.set_sync(sensor_id_secondary);
            REQUIRE(mock_hw.get_sync_state_mock() == true);
        }
    }
}

SCENARIO("Controling multiple sensors at once") {
    auto sync_control = sensors::hardware::SensorHardwareSyncControlSingleton();
    test_mocks::MockSensorHardware mock_hw_primary{version_wrapper, sync_control};
    test_mocks::MockSensorHardware mock_hw_secondary{version_wrapper, sync_control};
    GIVEN("Using the BOTH sensorid") {
        WHEN("BOTH sensors are enabled") {
            mock_hw_primary.set_sync_enabled(can::ids::SensorId::BOTH, true);
            mock_hw_secondary.set_sync_enabled(can::ids::SensorId::BOTH, true);
            THEN("Primary works") {
                mock_hw_primary.set_sync(sensor_id_primary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == true);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == true);
            }
            THEN("Secondary works") {
                mock_hw_secondary.set_sync(sensor_id_secondary);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == true);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == true);
            }
        }
        WHEN("BOTH sensors are required") {
            mock_hw_primary.set_sync_required(can::ids::SensorId::BOTH, true);
            mock_hw_secondary.set_sync_required(can::ids::SensorId::BOTH, true);
            THEN("Primary wont solo trigger") {
                mock_hw_primary.set_sync(sensor_id_primary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == false);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == false);
            }
            THEN("Secondary wont solo trigger") {
                mock_hw_secondary.set_sync(sensor_id_secondary);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == false);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == false);
            }
            THEN("both set triggers") {
                mock_hw_primary.set_sync(sensor_id_primary);
                mock_hw_secondary.set_sync(sensor_id_secondary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == true);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == true);
            }
        }
    }
    GIVEN("Using the NONE sensorid") {
        WHEN("NONE sensors are enabled") {
            mock_hw_primary.set_sync_enabled(can::ids::SensorId::UNUSED, true);
            mock_hw_secondary.set_sync_enabled(can::ids::SensorId::UNUSED, true);
            THEN("Primary doesn't set") {
                mock_hw_primary.set_sync(sensor_id_primary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == false);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == false);
            }
            THEN("Secondary doesn't set") {
                mock_hw_secondary.set_sync(sensor_id_secondary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == false);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == false);
            }
        }
        WHEN("NONE sensors are required") {
            mock_hw_primary.set_sync_required(can::ids::SensorId::UNUSED, true);
            mock_hw_secondary.set_sync_required(can::ids::SensorId::UNUSED, true);
            THEN("Primary wont solo trigger") {
                mock_hw_primary.set_sync(sensor_id_primary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == false);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == false);
            }
            THEN("Secondary wont solo trigger") {
                mock_hw_secondary.set_sync(sensor_id_secondary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == false);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == false);
            }
            THEN("both set wont trigger") {
                mock_hw_primary.set_sync(sensor_id_primary);
                mock_hw_secondary.set_sync(sensor_id_secondary);
                REQUIRE(mock_hw_primary.get_sync_state_mock() == false);
                REQUIRE(mock_hw_secondary.get_sync_state_mock() == false);
            }
        }
    }
}
