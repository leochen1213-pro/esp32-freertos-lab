# 學習筆記

## Lab 01

**Q：FreeRTOS 的排程規則是什麼？**
固定優先權、搶佔式：永遠執行 Ready 中優先權最高的 task。只要有更高優先權的 task 變成 Ready（tick 喚醒、建立、ISR 通知）就立刻切換。time slicing 只發生在相同優先權之間，不同優先權之間沒有公平性。

**Q：高優先權 task 不 block 會怎樣？怎麼發現？**
同核心上的低優先權 task 和 IDLE task 會被餓死。只能靠 task watchdog 監看 IDLE task。我實測過 Arduino 只監看 IDLE0：同一個 bug 在 core 1 完全安靜，在 core 0 觸發 watchdog 並 boot loop。

**Q：為什麼 `taskYIELD()` 修不了餓死？**
yield 只讓給優先權相同或更高的 Ready task。高優先權 task yield 後，排程器還是會選回它自己。要讓低優先權執行，必須進入 Blocked 狀態。

**Q：`vTaskDelay` 和 `vTaskDelayUntil` 差在哪？**
`vTaskDelay` 是相對延遲，從呼叫當下起算，工作時間跨過 tick 時週期會漂移。我量到 2 ms 工作加 `vTaskDelay(10)` 實際週期是 12 ms。`vTaskDelayUntil` 以上次喚醒時間為基準，週期固定。（詳見 Lab 02）

**Q：ESP32 有哪些 watchdog？**
Task watchdog 監看訂閱的 task（預設 IDLE task）是否定期執行，對應 Linux soft lockup；Interrupt watchdog 監看中斷是否被擋住，對應 hard lockup。Watchdog 是可設定的安全網，產品要自己決定監看誰、逾時多久。

**Q：同一份程式在 Arduino 和 ESP-IDF 上行為一樣嗎？**
不一定。我把同一個 starvation 實驗分別跑在兩邊：Arduino 不監看 IDLE1，core 1 的餓死完全安靜，core 0 則逾時約 15 s 後 panic 重開機；IDF 預設兩顆核心都監看，5 s 逾時但只印錯誤不重開。差異全來自預設設定，所以產品要明確設定 watchdog，不能依賴框架預設。

**Q：多核系統上，watchdog 怎麼拿到另一顆核心的 backtrace？**
偵測的核心讀不到另一顆核心的堆疊，所以送跨核心中斷，請對方自己印出 backtrace。我在 IDF 的 log 裡看到 `esp_crosscore_isr` 疊在 `high_task` 上面就是這個痕跡。Linux 用 IPI 做同樣的事，lockup 時會看到 `NMI backtrace for cpu X`。

**Q：`xTaskCreatePinnedToCore` 是標準 API 嗎？**
不是，是 Espressif 為雙核 SMP 加的擴充。官方 FreeRTOS V11 用 `xTaskCreateAffinitySet`。另外 ESP-IDF 的 stack 大小單位是 byte，原版是 word，跨平台移植要注意。

**Q：怎麼量 context switch 時間？**
用 RAM ring buffer 記錄時間戳，事後再印出以避免 probe effect。在 ESP32-S3 上量到高優先權 task 結束到低優先權開始約 10 µs（含量測開銷）。