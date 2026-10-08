// Lab 01 - Priority & starvation (ESP-IDF version, no Arduino)
// Step A: RUN_CORE 1
// Step B: RUN_CORE 0
// Step C: uncomment vTaskDelay(1) in high_task

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RUN_CORE 1

static void low_task(void *arg)
{
    for (;;) {
        printf("[low ] core %d, tick %" PRIu32 "\n", xPortGetCoreID(), (uint32_t)xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void high_task(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(3000));
    printf("[high] entering busy loop, never blocking\n");
    for (;;) {
        // busy work that never gives up the CPU
        vTaskDelay(1);   // Step C (note: 1 tick = 10 ms at IDF default 100 Hz)
    }
}

// Entry point. Runs in the "main" task, created by ESP-IDF startup code.
void app_main(void)
{
    printf("app_main on core %d, tick rate %d Hz\n", xPortGetCoreID(), configTICK_RATE_HZ);

    xTaskCreatePinnedToCore(low_task,  "low",  4096, NULL, 1, NULL, RUN_CORE);
    xTaskCreatePinnedToCore(high_task, "high", 4096, NULL, 5, NULL, RUN_CORE);

    // Returning is fine: the main task deletes itself after app_main returns.
}
