/***********************************************************
檔案名稱：ISO14443A_UID.ino
功能描述：
1. 使用 Serial4（硬體串列埠）與 BMC11T001 RFID 讀寫模組通訊，波特率 115200。
2. 使用 Serial（預設序列埠）與電腦進行監控輸出，波特率 9600。
3. 功能為：讀取 ISO14443A 類型卡片的 UID（唯一識別碼），並顯示於序列監控。
4. 本範例聚焦於 UID 讀取（尚未包含 EEPROM 寫入）。
硬體需求：模組插在 BMduino UNO 板上。
***********************************************************/

#include <String.h>             // 字串處理函式庫
#include "BMC11T001.h"          // 引入控制 BMC11T001 RFID 模組的函式庫

// 建立 BMC11T001 物件，指定通訊介面為 Serial4
BMC11T001 BMC11(&Serial4);

// 儲存 UID 長度的變數
int nlens;

// UID 資料的接收緩衝區（最多 50 bytes）
uint8_t uid_buf[50] = {0};

// 兩個字串變數用來存放卡號
String uidStr = "";       // 儲存數字型態 UID（例如轉為 long）
String uidByteStr = "";   // 儲存 16 進位字串型態 UID（例如：F0A1B2C3）
//-------------------------------
void initRFID() ;  // 初始化 RFID 讀卡模組
unsigned long UUIDString(int d4, int d3, int d2, int d1) ; // 將 4 個 byte 的 UID 組合成一個 32-bit 整數（常見卡號格式）
String readRFIDUUIDString() ;  // 將 UID（4 Bytes）轉為 HEX 字串，例如："F0A1B2C3"
// 將 UID（4 Bytes）轉為 HEX 字串，例如："F0A1B2C3"
String readRFIDUUIDString() ;  // 將 UID（4 Bytes）轉為 HEX 字串，例如："F0A1B2C3"

// 從 UID buffer 的 Byte 內容組成 HEX 字串，再轉換為常見數字型態 UID（補零）
String readRFIDUIDString() ; // 從 UID buffer 的 Byte 內容組成 HEX 字串，再轉換為常見數字型態 UID（補零）


//-------------------------------
// 初始化 RFID 讀卡模組
void initRFID()   // 初始化 RFID 讀卡模組
{
  delay(1000);                       // 等待模組啟動完成
  BMC11.begin(115200);              // 初始化 Serial4 並設定波特率為 115200
  BMC11.begin_ISO14443A();          // 啟動 ISO14443A 協定模式（如 MIFARE）
}

// 將 4 個 byte 的 UID 組合成一個 32-bit 整數（常見卡號格式）
unsigned long UUIDString(int d4, int d3, int d2, int d1)  // 將 4 個 byte 的 UID 組合成一個 32-bit 整數（常見卡號格式）
{
  // 利用位元加權方式組合
  unsigned long tmp = 0;
  tmp = d4 * (unsigned long)16777216 + d3 * (unsigned long)65536 + d2 * (unsigned long)256 + (unsigned long)d1;
  return tmp;
}

// 嘗試從 RFID 卡片讀取 UID，如成功回傳 true，否則 false
boolean checkReadRFIDSuccess() {
  nlens = BMC11.getUID_ISO14443A(uid_buf);  // 從 RFID 卡片讀取 UID，回傳長度

  // 檢查回傳長度是否為 12（特定標準格式）
  if (nlens == 12) {
    return true;
  } else {
    return false;
  }
}

// 讀取 RFID UID，轉換為整數字串，並補足至 10 位數（不足補零）
String readRFIDUUID() {
  String tmp = "";
  unsigned long ttmp;

  // 透過 UUIDString() 將 UID 四個 byte 組合為整數
  ttmp = UUIDString(
    uid_buf[3],
    uid_buf[2],
    uid_buf[1],
    uid_buf[0]
  );

  tmp = String(ttmp);  // 轉為字串
  Serial.print("tmp is :("); Serial.print(tmp); Serial.println(")");

  int len = tmp.length();
  Serial.print("Len is :("); Serial.print(len); Serial.println(")");

  if (len < 10) {
    return genstr('0', 10 - len) + tmp;  // 補零使長度為 10
  } else {
    return tmp;
  }
  return tmp;
}

// 將 UID（4 Bytes）轉為 HEX 字串，例如："F0A1B2C3"
String readRFIDUUIDString()   // 將 UID（4 Bytes）轉為 HEX 字串，例如："F0A1B2C3"
{
  String uidString = "";

  // 從 uid_buf 的第 3~0 個 byte 轉為 HEX（高位在前）
  for (int i = 3; i >= 0; i--) {
    uidString += print2HEX((int)uid_buf[i]);  // 每 byte 轉成兩位 HEX 串
  }

  uidString.toUpperCase(); // 統一轉為大寫
  return uidString;
}

// 從 UID buffer 的 Byte 內容組成 HEX 字串，再轉換為常見數字型態 UID（補零）
String readRFIDUIDString()  // 從 UID buffer 的 Byte 內容組成 HEX 字串，再轉換為常見數字型態 UID（補零）
{
  String tmp = "";               // 回傳最終結果
  unsigned long ttmp;            // 暫存整數型 UID
  String uidString = "";         // 暫存 HEX 字串 UID
  int d1, d2, d3, d4;            // 四個 byte 對應的整數值
  String s1, s2, s3, s4;         // 各 byte HEX 字串

  // 將 uid_buf 的第 4~11 byte（共 8 個）轉成字串（以 char 表示）
  for (int i = 4; i < 12; i++) {
    uidString += String(char(uid_buf[i]));
  }

  uidString.toUpperCase(); // 統一轉大寫

  // 從字串中取出每兩位作為一個 byte 的 HEX 字串
  s1 = uidString.substring(0, 2);
  s2 = uidString.substring(2, 4);
  s3 = uidString.substring(4, 6);
  s4 = uidString.substring(6, 8);

  // 將 HEX 字串轉為十進位整數（strtoul：字串轉無號長整數）
  d1 = (int)strtoul(s1.c_str(), NULL, 16);
  d2 = (int)strtoul(s2.c_str(), NULL, 16);
  d3 = (int)strtoul(s3.c_str(), NULL, 16);
  d4 = (int)strtoul(s4.c_str(), NULL, 16);

  // 將四個 byte 組合成一個長整數
  ttmp = UUIDString(d4, d3, d2, d1);
  tmp = String(ttmp);

  Serial.print("tmp is :("); Serial.print(tmp); Serial.println(")");
  int len = tmp.length();
  Serial.print("Len is :("); Serial.print(len); Serial.println(")");

  // 若長度不足 10 位數，自動補 0
  if (len < 10) {
    return genstr('0', 10 - len) + tmp;
  } else {
    return tmp;
  }
  return tmp;
}
