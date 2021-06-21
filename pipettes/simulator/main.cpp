#include <boost/thread/thread.hpp>

#include "FreeRTOS.h"
#include "comm.h"
#include "task.h"


int main() {
    comm c;
    boost::thread t(c);
    t.join();

    vTaskStartScheduler();
    return 0;
}