# esp32-freertos-lab

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

## 新增一個 lab

```bash
mkdir -p labs/labNN-<topic>/{idf,logs}
cp -r templates/idf-project labs/labNN-<topic>/idf/<project>
cp templates/lab-template.md labs/labNN-<topic>/README.md
```

