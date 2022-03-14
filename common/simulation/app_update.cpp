#include "common/core/app_update.h"

#include "common/core/logging.h"

void app_update_start() { LOG("Starting app update!"); }

update_flag_type app_update_flags() { return 0; }