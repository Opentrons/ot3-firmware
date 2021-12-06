#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/errors.h"
#include "common/firmware/adc.hpp"


//call adc config methos
//call adc read method from free RTOS task




[[noreturn]] void task_entry() {
    adc_setup();
    //call function that reads voltage here!
    
}

auto static task = FreeRTOSTask<512, 5>("can task", task_entry);