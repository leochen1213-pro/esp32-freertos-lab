# Lab 01：優先權、搶佔與餓死

## 目標
- FreeRTOS 怎麼決定下一個執行的 task？
- 高優先權 task 不 block 會發生什麼事？系統怎麼發現？
- 怎麼在不干擾時序的情況下，看清楚每個時間點是誰在跑？

## 實驗
| 專案 | 說明 |
|---|---|
| `arduino/w1_starvation/` | Step A/B/C：餓死、watchdog、修正 |
| `arduino/w1_highcount/` | 用計數器證明 high 每個 tick 都在執行 |
| `arduino/w1_trace/` | RAM ring buffer tracer，量測搶佔與 context switch |
| `idf/starvation/` | 同一個 Step A/B/C 改用 ESP-IDF，比較預設行為 |

### 步驟
1. **Step A**（`RUN_CORE 1`）：high（優先權 5）進入 busy loop，從不 block。
2. **Step B**（`RUN_CORE 0`）：同樣的程式碼改在 core 0 執行。
3. **Step C**：在 busy loop 裡加上 `vTaskDelay(1)`。
4. **highcount**：high 每次醒來 `highCount++`，由 low 印出。
5. **trace**：low 每次忙 2 ms、high 每 tick 忙 0.3 ms，記錄時間戳後再一次印出。

## 結果
| 實驗 | 現象 | Log |
|---|---|---|
| Step A | high 開始後 low 完全消失，**沒有任何錯誤訊息** | `logs/arduino/step_a_core1_silent.log` |
| Step B | 約 15 s 後 task watchdog 觸發（`IDLE0`、`CPU 0: high`），panic 重開機，陷入 **boot loop** | `logs/arduino/step_b_core0_watchdog.log` |
| Step C | low 恢復，每 500 ms 正常印出 | `logs/arduino/step_c_fixed.log` |
| highcount | `highCount` 每 500 ms 剛好 +500，即每 tick 恰好 +1 | `logs/arduino/highcount.log` |
| trace | high 每 1000 µs 醒來；low 的週期是 12 ms 而非 10 ms | `logs/arduino/trace.log` |
| IDF Step A | core 1 的餓死**被抓到**（`IDLE1`），5 s 逾時，只印錯誤、每 5 s 重複，不重開機 | `logs/idf/step_a_core1_watchdog.log` |

### 實測數據
- Tick 週期 1000 µs，抖動約 1 µs。
- Context switch（含 trace 開銷）約 10 µs（high end 11765 → low start 11776）。
- Arduino task watchdog 逾時約 15 s。

## Arduino vs ESP-IDF

同一份 Step A 程式碼，換框架後行為不同，差異全來自預設設定：

| | Arduino | ESP-IDF v6.1 預設 |
|---|---|---|
| Tick rate | 1000 Hz | 100 Hz（tick 每 500 ms 只加 50） |
| 監看 IDLE1 | 否 → core 1 餓死完全安靜 | 是 → 被抓到 |
| Watchdog 逾時 | 約 15 s | 5 s |
| 觸發後 | panic 重開機，boot loop | 只印錯誤，每 5 s 重複 |
| Backtrace | 只有位址，手動 `addr2line` | `idf.py monitor` 自動解析成函式與行號 |
| 進入點 | `setup()`/`loop()` 跑在 core 1 的 loopTask | `app_main()` 跑在 core 0 的 main task，return 後該 task 被刪除 |

**結論：watchdog 是可設定的安全網，不是系統自帶的保證。** Arduino 把這些設定藏起來，IDF 用 menuconfig 全部攤開。

### 讀懂 IDF 的 backtrace（由下往上）
- `vPortTaskWrapper`：FreeRTOS 啟動每個 task 的包裝函式。
- `high_task at main.c:25`：兇手。空的無窮迴圈只剩一個跳回自己的指令，除錯資訊可能對應到相鄰行。
- `_xt_lowint1` → `esp_crosscore_isr`：core 0 偵測到 IDLE1 逾時，但讀不到 core 1 的堆疊，於是送**跨核心中斷**請 core 1 自己印出 backtrace。對應 Linux lockup 時用 IPI 要求各 CPU 印出 `NMI backtrace for cpu X`。

## 學到什麼

### 排程規則
- **固定優先權、搶佔式**：永遠執行 Ready 中優先權最高的 task。
- **只要有更高優先權的 task 變成 Ready 就立刻搶佔**，不必等 tick：high 建立後 22 µs 就搶走 CPU。
- **沒有跨優先權的公平性**：time slicing 只發生在相同優先權之間。
- **同一個 tick 醒來時高優先權先跑**：tick 4505、11462 都觀察到 low 要等 high 做完。

### Task 狀態
- `vTaskDelay()` 讓 task 進入 **Blocked**，低優先權才有機會執行。
- `taskYIELD()` 救不了餓死：只會讓給優先權相同或更高的 task。
- `vTaskDelay(1)` 只是實驗修補；正確設計是等待事件（queue、semaphore、notification）。

### 餓死與 watchdog
- 餓死是**設計 bug**，不是 runtime error：排程器完全照規則運作。
- 同一個 bug：core 1 上安靜（Arduino 不監看 IDLE1），core 0 上 boot loop（監看 IDLE0）。
- Task watchdog 監看 IDLE task 是否有機會執行；可用 `enableCore1WDT()`、`esp_task_wdt_add()` 擴大監看範圍。
- Interrupt watchdog 監看中斷是否被擋住；Step A 沒關中斷，所以不會觸發。
- 單核晶片（如 ESP32-C3）上同樣的 bug 會拖垮整個系統；雙核時另一顆核心照常運作。
- 重開機原因：`0x1 POWERON` 是上電，`0xc RTC_SW_CPU_RST` 是軟體觸發（panic）。

### 時間
- Arduino tick rate 1000 Hz（ESP-IDF 預設 100 Hz）；`xTaskGetTickCount()` ≈ Linux jiffies。
- `pdMS_TO_TICKS()` 只是單位換算，真正睡覺的是 `vTaskDelay()`；整數除法會捨去，精度受限於 tick。
- `vTaskDelay` 是**相對延遲**，從呼叫當下的 tick 起算：
  - 工作時間不到 1 tick（printf 進緩衝區）→ 被 tick 粒度吸收，週期穩定 500 ms。
  - 工作時間跨過 tick（2 ms spin）→ 週期漂成 2 + 10 = 12 ms。→ Lab 02
- Wall-clock time ≠ CPU time：「忙等到某時間點」的工作被搶佔時會縮水。

### Trace
- 先寫 RAM buffer、事後由另一顆核心印出，避免 **probe effect**（與 ftrace ring buffer 同思路）。
- 共用計數器只有一個 writer 時不需保護；多個 writer 做 `++` 會有 race condition。→ Lab 04

### ESP32 特有
- `xTaskCreatePinnedToCore` 是 Espressif 擴充，非標準 FreeRTOS API（官方 V11 SMP 用 `xTaskCreateAffinitySet`）。
- ESP-IDF 的 stack 大小單位是 **byte**，原版 FreeRTOS 是 **word**。
- SMP 下餓死與否也取決於 core affinity：不釘核心的 task 可以跑到另一顆核心。

## 推翻的預測
| 原本以為 | 實際上 |
|---|---|
| Arduino watchdog 逾時 5 s | 約 15 s（5 s、只印警告其實是 **IDF** 的預設，已在 IDF 上驗證） |
| Arduino 預設只印警告 | panic 重開機，形成 boot loop |
| low 的 2 ms 工作被搶佔後會拉長到約 2.6 ms | `spin_us()` 是忙等到時間點，工作縮水，仍是 2 ms |

## 對照 Linux
| 情境 | FreeRTOS | Linux |
|---|---|---|
| 一般 task 跑 busy loop | 低優先權被完全餓死 | EEVDF 公平分配，不會餓死 |
| 偵測霸佔 CPU | Task watchdog 監看 IDLE task | soft lockup 偵測 |
| 中斷被擋住 | Interrupt watchdog | hard lockup 偵測（NMI watchdog） |
| 最接近的行為 | 固定優先權搶佔 | `SCHED_FIFO`（有 RT throttling 保留約 5%） |
| 讓出 CPU | `vTaskDelay()`、等待事件 | `cond_resched()`、`wait_event()`、`msleep()` |
| ms → tick | `pdMS_TO_TICKS()` | `msecs_to_jiffies()` |
| Task stack | 建立時指定，ESP-IDF 以 byte 計 | kernel stack 8 KB（ARM32）／16 KB（ARM64） |
| 綁定核心 | `xTaskCreatePinnedToCore` | `sched_setaffinity()`、`taskset` |
| 跨核心要求 backtrace | crosscore interrupt | IPI（NMI backtrace） |

## 面試 30 秒
> FreeRTOS 是固定優先權搶佔，不保證公平。高優先權 task 如果不 block，低優先權就會被餓死，只能靠 watchdog 監看 IDLE task 來發現。我在 ESP32-S3 上重現過：同一個 bug 在沒被監看的核心上完全安靜，在被監看的核心上則造成 boot loop。我也用 ring buffer 做了簡易 tracer，量到 tick 抖動約 1 µs、context switch 約 10 µs。Linux 一般排程講求公平，但 kernel 程式碼不讓出 CPU 一樣會觸發 soft lockup，`SCHED_FIFO` 則是和 FreeRTOS 行為最接近的情況。

## 參考
- Mastering the FreeRTOS Real Time Kernel：Task Management 章節
