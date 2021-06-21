#include <boost/thread/thread.hpp>

#include "FreeRTOS.h"
#include "comm.h"
#include "task.h"

int main() {
    boost::thread t{comm()};
    t.join();

    vTaskStartScheduler();
    return 0;
}