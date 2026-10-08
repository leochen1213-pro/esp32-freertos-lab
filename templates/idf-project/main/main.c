// Lab NN - <topic>

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("app_main on core %d, tick rate %d Hz\n",
           xPortGetCoreID(), configTICK_RATE_HZ);
}
