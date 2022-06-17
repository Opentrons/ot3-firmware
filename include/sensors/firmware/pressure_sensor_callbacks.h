#include "platform_specific_hal_conf.h"

typedef void(*sensor_callback) ();
sensor_callback data_ready_callback;
//static void * data_ready_callback;
void drdy_callback();
