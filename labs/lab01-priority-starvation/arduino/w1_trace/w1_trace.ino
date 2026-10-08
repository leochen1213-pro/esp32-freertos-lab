// Week 1 - Trace: see preemption with your own eyes
// Tasks record events (timestamp in us) into a RAM buffer.
// A separate task on the other core prints the buffer once it is full,
// so printing does not disturb the timing being measured.

#define RUN_CORE 0
#define MAX_EVT  64

struct Evt { uint32_t t_us; char who; char what; };
static Evt evts[MAX_EVT];
static volatile int nEvt = 0;
static portMUX_TYPE traceMux = portMUX_INITIALIZER_UNLOCKED;

static void trace(char who, char what) {
  taskENTER_CRITICAL(&traceMux);          // protect nEvt between tasks/cores
  if (nEvt < MAX_EVT) {
    evts[nEvt].t_us = (uint32_t)esp_timer_get_time();
    evts[nEvt].who  = who;
    evts[nEvt].what = what;
    nEvt++;
  }
  taskEXIT_CRITICAL(&traceMux);
}

static void spin_us(uint32_t us) {        // burn CPU for 'us' microseconds
  int64_t end = esp_timer_get_time() + us;
  while (esp_timer_get_time() < end) { }
}

void lowTask(void *arg) {
  for (;;) {
    trace('L', 'S');
    spin_us(2000);                        // 2 ms of work, longer than 1 tick
    trace('L', 'E');
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void highTask(void *arg) {
  for (;;) {
    trace('H', 'S');
    spin_us(300);                         // 0.3 ms of work
    trace('H', 'E');
    vTaskDelay(1);                        // block until next tick
  }
}

void dumpTask(void *arg) {
  while (nEvt < MAX_EVT) vTaskDelay(pdMS_TO_TICKS(10));
  uint32_t t0 = evts[0].t_us;
  Serial.println("   t(us)  task  event");
  for (int i = 0; i < MAX_EVT; i++) {
    Serial.printf("%8lu   %s  %s\n",
                  (unsigned long)(evts[i].t_us - t0),
                  evts[i].who == 'H' ? "high" : "low ",
                  evts[i].what == 'S' ? "start" : "end");
  }
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  xTaskCreatePinnedToCore(lowTask,  "low",  4096, NULL, 1, NULL, RUN_CORE);
  xTaskCreatePinnedToCore(highTask, "high", 4096, NULL, 5, NULL, RUN_CORE);
  xTaskCreatePinnedToCore(dumpTask, "dump", 4096, NULL, 1, NULL, 1 - RUN_CORE);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
