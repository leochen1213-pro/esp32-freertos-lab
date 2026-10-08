// Lab 01 - Prove highTask runs every tick (Step C + counter)
// highCount is written only by highTask and read by lowTask.

#define RUN_CORE 0

static volatile uint32_t highCount = 0;

void lowTask(void *arg) {
  for (;;) {
    Serial.printf("[low ] tick %lu, highCount %lu\n",
                  (unsigned long)xTaskGetTickCount(), (unsigned long)highCount);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void highTask(void *arg) {
  vTaskDelay(pdMS_TO_TICKS(3000));
  Serial.println("[high] entering busy loop, never blocking");
  for (;;) {
    highCount++;
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  xTaskCreatePinnedToCore(lowTask,  "low",  4096, NULL, 1, NULL, RUN_CORE);
  xTaskCreatePinnedToCore(highTask, "high", 4096, NULL, 5, NULL, RUN_CORE);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
