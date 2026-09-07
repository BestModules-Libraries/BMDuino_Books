/***********************************************************
 File:                ISO14443A_UID_hex.ino
 Description:         1. 使用 HardwareSerial4 (115200bps) 與 BMC11T001 通訊。
                      2. 使用 Serial (115200bps) 與序列埠監視器顯示訊息。
                      功能：讀取 ISO14443A 卡片 UID，若有偵測到卡片，
                           以「十六進位、人類可讀格式」顯示 UID；
                           若無卡片則顯示提示文字。
 connection method:   直接插上 BMduino UNO（請確認板上是否具備 Serial4）
***********************************************************/

#include "BMC11T001.h"

// 建立 BMC11 物件，指定以 Serial4 與模組通訊
BMC11T001 BMC11(&Serial4);

// 讀卡用暫存緩衝區與長度
int nlens = 0;                   // 讀到的 UID 長度（位元組數）
uint8_t uid_buf[50] = {0};       // UID 暫存緩衝區（依需求可調整大小）

// ---- 小工具函式：以「兩位十六進位」輸出一個位元組（前綴補 0）----
void printByteAsHex(uint8_t b)
{
  if (b < 0x10) Serial.print("0"); // 小於 0x10 補 0，維持兩位
  Serial.print(b, HEX);
}

// ---- 小工具函式：以「十六進位字串（含空格分隔）」輸出整個 UID ----
void printUIDHex(const uint8_t *buf, int len)
{
  Serial.print("UID (HEX): ");
  for (int i = 0; i < len; i++) {
    printByteAsHex(buf[i]);
    if (i < len - 1) Serial.print(" "); // 位元組間加空格
  }
  Serial.println();
}

void setup() 
{
  delay(1000);                    // 等 BMC11T001 完成上電初始化

  // 與 BMC11T001 模組通訊的鮑率（對 Serial4）
  BMC11.begin(115200);

  // 與電腦序列監視器的鮑率（對 USB Serial）
  // 建議與模組同速率，避免你誤以為亂碼
  Serial.begin(9600);
  //while (!Serial) { ; }           // 等待序列埠就緒（某些板子需要）

  Serial.println("=== ISO14443A UID Reader (HEX Display) ===");
  Serial.println("Baudrate: 115200 (Serial & Serial4)");
  Serial.println("Bring an ISO14443A card close to the reader...");
  Serial.println();

  // 啟用模組的 ISO14443A 模式（Type A：如 MIFARE、NTAG 等）
  BMC11.begin_ISO14443A();
}

void loop()
{
  // 嘗試讀取卡片 UID，將資料寫入 uid_buf，並回報長度到 nlens
  nlens = BMC11.getUID_ISO14443A(uid_buf);

  if (nlens > 0) {
    // 讀到卡片：以十六進位格式顯示 UID
    printUIDHex(uid_buf, nlens);

    // 若你同時也需要「原始位元組」輸出，可取消下行註解：
    // Serial.write(uid_buf, nlens), Serial.println();

  } else {
    // 沒讀到卡片：印出提示（避免空白輸出讓人以為當機）
    Serial.println("No ISO14443A card detected.");
  }

  delay(500); // 稍作休息，避免輪詢過快；可依需求調整
}
