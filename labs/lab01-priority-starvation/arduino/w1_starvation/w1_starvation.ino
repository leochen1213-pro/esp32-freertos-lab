// Lab 01 - Priority & starvation
// Step A: run as-is (both tasks on core 1)        -> silent starvation
// Step B: change RUN_CORE to 0                     -> task watchdog, boot loop
// Step C: uncomment the vTaskDelay(1) in highTask  -> both tasks run

#define RUN_CORE 1

void lowTask(void *arg) {
  for (;;) {
    Serial.printf("[low ] core %d, tick %lu\n",
                  xPortGetCoreID(), (unsigned long)xTaskGetTickCount());
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void highTask(void *arg) {
  vTaskDelay(pdMS_TO_TICKS(3000));      // let lowTask run for 3 s first
  Serial.println("[high] entering busy loop, never blocking");
  for (;;) {
    // busy work that never gives up the CPU
    // vTaskDelay(1);                   // Step C: uncomment this line
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  xTaskCreatePinnedToCore(lowTask,  "low",  4096, NULL, 1, NULL, RUN_CORE);
  xTaskCreatePinnedToCore(highTask, "high", 4096, NULL, 5, NULL, RUN_CORE);
}

void loop() {
  vTaskDelay(portMAX_DELAY);            // loopTask just sleeps
}
