# FreeRTOS ↔ Linux 對照

每個 lab 完成後，把新的對照補進來。

| 概念 | FreeRTOS | Linux kernel | 來源 |
|---|---|---|---|
| 執行單位 | task（共用位址空間，無 MMU） | kernel thread | Lab 01 |
| 排程 | 固定優先權搶佔 | EEVDF（一般）、`SCHED_FIFO`（即時） | Lab 01 |
| 時間單位 | tick，`xTaskGetTickCount()` | jiffies | Lab 01 |
| 單位換算 | `pdMS_TO_TICKS()` | `msecs_to_jiffies()` | Lab 01 |
| 主動讓出 CPU | `vTaskDelay()`、等待事件 | `cond_resched()`、`msleep()`、`wait_event()` | Lab 01 |
| 霸佔 CPU 偵測 | Task watchdog | soft lockup | Lab 01 |
| 中斷被擋偵測 | Interrupt watchdog | hard lockup（NMI watchdog） | Lab 01 |
| 核心綁定 | `xTaskCreatePinnedToCore` | `sched_setaffinity()` | Lab 01 |
| Tracing | 自製 RAM ring buffer → SystemView | ftrace | Lab 01 |
| 跨核心通知 | crosscore interrupt（如 watchdog 要求另一核印 backtrace） | IPI | Lab 01 |
| 組態系統 | Kconfig：`idf.py menuconfig` | Kconfig：`make menuconfig` | Lab 01 |
| 組態檔 | `sdkconfig.defaults`（進版控）→ `sdkconfig` | `defconfig`（進版控）→ `.config` | Lab 01 |
| 硬體描述 | ESP-IDF：寫在 C 程式碼（如 `i2c_master_bus_config_t`）；Zephyr：device tree | device tree | Lab 01 |
