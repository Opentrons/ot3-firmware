#pragma once
#ifndef SENSOR_BUFF_SIZE
#define SENSOR_BUFFER_SIZE = 0;
#else
constexpr size_t SENSOR_BUFFER_SIZE = SENSOR_BUFF_SIZE;
#endif