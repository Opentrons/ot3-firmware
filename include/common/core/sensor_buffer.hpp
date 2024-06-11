#pragma once

#ifdef USE_SENSOR_MOVE
constexpr size_t SENSOR_BUFFER_SIZE = SENSOR_BUFF_SIZE;
#else
constexpr size_t SENSOR_BUFFER_SIZE = 0;
#endif