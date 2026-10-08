# esp32-freertos-lab

在 ESP32-S3 上用實驗學 FreeRTOS。每個觀念都「先做壞、再修好」，記錄實測數據，並對照 Linux kernel 的對應機制。

Lab 01 用 Arduino 起步，之後全部改用 **ESP-IDF**。

## 硬體與環境

| 項目 | 內容 |
|---|---|
| 開發板 | ESP32-S3（N16R8：16 MB flash、8 MB octal PSRAM） |
| 主機 | macOS |
| 主要框架 | ESP-IDF v6.1（透過 EIM 安裝） |
| 輔助 | Arduino IDE 2.3.10（Lab 01、快速測試） |
| 共用設定 | `CONFIG_FREERTOS_HZ=1000`、`CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y` |

> 開啟 OPI PSRAM 後，GPIO 35/36/37 被佔用，接線時避開。

環境建置步驟與踩過的坑：[docs/toolchain.md](docs/toolchain.md)

## 建置與燒錄

```bash
get_idf                                  # 載入 IDF 環境（alias，見 toolchain.md）
cd labs/labNN-<topic>/idf/<project>
idf.py set-target esp32s3                # 每個專案第一次執行
idf.py build
idf.py -p /dev/cu.usbserial-XX flash monitor   # Ctrl-] 離開 monitor
```

## Labs

| Lab | 主題 | 框架 | 狀態 |
|---|---|---|---|
| [01](labs/lab01-priority-starvation/) | 優先權、搶佔、餓死與 watchdog | Arduino + IDF | ✅ |
| 02 | `vTaskDelay` vs `vTaskDelayUntil`：週期漂移 | IDF | ⬜ |
| 03 | Queue：producer-consumer（+ 讀 `xQueueReceive` 原始碼） | IDF | ⬜ |
| 04 | Race condition 與 mutex | IDF | ⬜ |
| 05 | Priority inversion 與 priority inheritance（+ 讀 `xTaskPriorityInherit` 原始碼） | IDF | ⬜ |
| 06 | GPIO 中斷 + task notification（deferred work） | IDF | ⬜ |
| 07 | Software timer | IDF | ⬜ |
| 08 | Stack overflow、heap、deadlock | IDF | ⬜ |
| 09 | 觀測工具：runtime stats、`vTaskList`、core dump | IDF | ⬜ |
| — | 專題：多 task 系統 + 自寫 I2C driver | IDF | ⬜ |

## 目錄結構

```
esp32-freertos-lab/
├── README.md
├── labs/
│   └── labNN-<topic>/
│       ├── README.md          # 目標、步驟、結果、學到什麼、Linux 對照
│       ├── idf/<project>/     # ESP-IDF 專案（從 templates/idf-project 複製）
│       │   ├── CMakeLists.txt
│       │   ├── sdkconfig.defaults
│       │   └── main/
│       ├── arduino/<sketch>/  # 僅 Lab 01
│       └── logs/              # 原始 Serial 輸出
├── docs/
│   ├── toolchain.md           # 環境建置與踩坑紀錄
│   ├── freertos-vs-linux.md   # 累積的 FreeRTOS ↔ Linux 對照
│   └── learn-notes.md         
└── templates/
    ├── lab-template.md        # 新 lab 的 README 範本
    └── idf-project/           # 新 lab 的 IDF 專案骨架
```

### 關於 sdkconfig
- `sdkconfig.defaults` 進版控：只記錄「和預設值不同」的設定，相當於 kernel 的 defconfig。
- `sdkconfig` 不進版控：由 `idf.py` 依 defaults 自動產生，相當於 kernel 的 `.config`。
- 用 `idf.py menuconfig` 試出想要的設定後，把對應的 `CONFIG_...` 行補進 `sdkconfig.defaults`。

## 新增一個 lab

```bash
mkdir -p labs/labNN-<topic>/{idf,logs}
cp -r templates/idf-project labs/labNN-<topic>/idf/<project>
cp templates/lab-template.md labs/labNN-<topic>/README.md
```

## 相關專案

- bbb-kernel-labs：Bootlin Linux kernel & driver 課程（BeagleBone Black）
