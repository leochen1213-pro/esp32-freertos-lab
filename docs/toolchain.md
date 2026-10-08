# 環境建置紀錄（macOS + ESP-IDF v6.1）

## 安裝步驟

```bash
# 1. 相依套件
brew install libgcrypt glib pixman sdl2 libslirp dfu-util cmake python

# 2. EIM（ESP-IDF Installation Manager）
brew tap espressif/eim
brew trust --formula espressif/eim/eim
brew install eim

# 3. 安裝最新穩定版 ESP-IDF（自訂版本或路徑用 eim wizard）
eim install

# 4. 設定啟用 alias（腳本必須 source，不能直接執行）
echo "alias get_idf='. ~/.espressif/tools/activate_idf_v6.1.sh'" >> ~/.zshrc
source ~/.zshrc

# 5. 確認
get_idf
idf.py --version
```

每開一個新的終端機，都要先執行 `get_idf`。

## 踩過的坑

| 症狀 | 原因 | 解法 |
|---|---|---|
| `Refusing to load formula ... from untrusted tap` | 新版 Homebrew 預設不信任第三方 tap | `brew trust --formula espressif/eim/eim` |
| `Your Command Line Tools are too outdated` | Apple CLT 版本太舊 | `softwareupdate --list` 找到 CLT 的 Label，再 `sudo softwareupdate -i "<Label>"` |
| `xcode-select --install` 說已安裝 | 已安裝舊版時不會重裝 | 用上面的 `softwareupdate`，或先刪 `/Library/Developer/CommandLineTools` 再裝 |
| `Could not open /dev/cu.usbmodem...` | 換了 USB 孔或重插後 port 名稱改變 | `ls /dev/cu.*` 找目前名稱 |
| `Failed to connect to ESP32-S3: No serial data received` | 板子沒進入燒錄模式 | 按住 BOOT → 按一下 RST → 放開 BOOT，再燒錄 |
| Port busy | Arduino Serial Monitor 或 `screen` 佔用 | 先關掉其他 serial 工具 |
| `--flash-size 2MB` | IDF 預設值 | `sdkconfig.defaults` 加 `CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y` |

## Port 名稱
板子上的兩個 USB-C 孔在 macOS 會出現不同名稱（`cu.usbmodem...`、`cu.usbserial-...`），兩個都能燒錄與看輸出。

## Arduino IDE 設定（Lab 01）
- Board：ESP32S3 Dev Module（不是 Octal WROOM2）
- Flash Size：16MB、PSRAM：OPI PSRAM
- USB CDC On Boot：插 UART 孔時 Disabled，插 USB 孔時 Enabled
