/*
 * 檔案名稱：RFIDLib.h
 * 程式語言：Arduino C/C++
 * 檔案類型：標頭檔（Header File）
 * 
 * ========================================
 * 程式功能完整解說
 * ========================================
 * 
 * 【檔案概述】
 *   本檔案為 RFID（無線射頻識別）讀卡模組的驅動程式庫，專門用於控制
 *   BMC11T001 RFID 讀寫模組。主要功能是讀取 ISO14443A 類型卡片
 *   （如 MIFARE 系列卡片）的 UID（唯一識別碼），並提供多種格式轉換功能，
 *   方便後續的門禁權限驗證或資料記錄使用。
 * 
 * 【硬體規格】
 *   - RFID 模組：BMC11T001（支援 ISO14443A 協定）
 *   - 通訊介面：Serial4（硬體串列埠）
 *   - 通訊波特率：115200 bps
 *   - 適用卡片：MIFARE Classic、MIFARE Ultralight 等 ISO14443A 卡片
 *   - 開發板：BMduino UNO 或 Arduino 相容開發板
 * 
 * 【主要功能】
 *   1. 初始化 RFID 模組（initRFID）
 *   2. 偵測是否有卡片靠近（checkReadRFIDSuccess）
 *   3. 讀取卡片 UID 並轉換為多種格式：
 *      - 10 位數整數字串（例如："0007928640"）
 *      - HEX 字串（例如："F0A1B2C3"）
 *      - 32 位元無號整數（unsigned long）
 *   4. 自動補零功能，確保輸出格式一致
 * 
 * 【UID 資料格式說明】
 *   ISO14443A 卡片的 UID 通常為 4 個位元組（Bytes）或 7 個位元組，
 *   本程式主要處理 4 位元組的 UID 格式。
 *   
 *   UID 原始資料（uid_buf 陣列）：
 *     - 成功讀取後，uid_buf[0]~uid_buf[3] 儲存 UID 的 4 個位元組
 *     - 位元組順序為：uid_buf[3]（最高位元組）、uid_buf[2]、uid_buf[1]、uid_buf[0]（最低位元組）
 * 
 *   UID 轉換範例：
 *     原始 UID：0x12, 0x34, 0x56, 0x78
 *     - HEX 字串格式："12345678"
 *     - 整數格式：0x12345678 = 305419896
 *     - 10 位數字串："0305419896"（補零至 10 位）
 * 
 * 【運作流程】
 *   1. 初始化階段：
 *      a. 呼叫 initRFID() 函式
 *      b. 延遲 1 秒等待模組啟動
 *      c. 啟動 Serial4 通訊，波特率 115200
 *      d. 呼叫 begin_ISO14443A() 啟動 ISO14443A 協定
 * 
 *   2. 卡片偵測階段：
 *      a. 呼叫 checkReadRFIDSuccess() 函式
 *      b. 呼叫 BMC11.getUID_ISO14443A(uid_buf) 讀取 UID
 *      c. 檢查回傳長度是否為 12（成功讀取標誌）
 *      d. 回傳 true/false 表示是否有卡片
 * 
 *   3. UID 讀取與轉換階段：
 *      a. 呼叫 readRFIDUIDString() 或 readRFIDUUIDString()
 *      b. 從 uid_buf 中提取 UID 資料
 *      c. 根據需要轉換為 HEX 字串或 10 位數字串
 *      d. 回傳格式化後的字串
 * 
 * 【相依函式庫】
 *   - commlib.h：提供通用字串處理函式（如 genstr、print2HEX 等）
 *   - String.h：Arduino 內建字串處理函式庫
 *   - BMC11T001.h：BMC11T001 模組的底層驅動程式庫
 * 
 * 【全域變數說明】
 *   - BMC11：BMC11T001 類別物件，用於與 RFID 模組通訊
 *   - nlens：儲存 UID 讀取回傳的長度值
 *   - uid_buf：UID 資料接收緩衝區（50 bytes）
 *   - uidByteStr：儲存 HEX 格式的 UID 字串
 * 
 * 【注意事項】
 *   - 使用前需確認 BMC11T001 模組正確連接至 Serial4 腳位
 *   - 序列埠設定必須與模組匹配（115200 bps）
 *   - 不同卡片類型的 UID 長度可能不同，本程式針對 4 Bytes UID 設計
 *   - checkReadRFIDSuccess() 回傳 true 時，uid_buf 才包含有效資料
 *   - 讀取卡片時需將卡片靠近模組天線區域
 * 
 * 【版本資訊】
 *   最後修改日期：2026年3月27日
 *   適用專案：註冊RFID裝置之雲端門禁裝置
 */

/***********************************************************
 * 原始檔案名稱：ISO14443A_UID.ino
 * 功能描述：
 *   1. 使用 Serial4（硬體串列埠）與 BMC11T001 RFID 讀寫模組通訊，波特率 115200
 *   2. 使用 Serial（預設序列埠）與電腦進行監控輸出，波特率 9600
 *   3. 功能為：讀取 ISO14443A 類型卡片的 UID（唯一識別碼），並顯示於序列監控
 *   4. 本範例聚焦於 UID 讀取（尚未包含 EEPROM 寫入）
 * 硬體需求：模組插在 BMduino UNO 板上
 ***********************************************************/

// ========================================
// NFC RFID 使用的函式庫，BMC11T001 RFID 讀寫模組通訊
// ========================================

//---------外部函式庫引入區--------
#include <commlib.h>             // 引入通用通訊函式庫
                                 // 提供 genstr()、print2HEX() 等字串處理輔助函式
                                 // genstr('0', n)：產生 n 個 '0' 組成的字串
                                 // print2HEX(value)：將數值轉為兩位 HEX 字串

#include <String.h>             // 引入 Arduino 內建字串處理函式庫
                                 // 提供 String 類別及相關字串操作方法

#include "BMC11T001.h"          // 引入 BMC11T001 RFID 模組控制函式庫
                                 // 提供底層通訊功能，包含 ISO14443A 協定的 UID 讀取

// ========================================
// 全域變數定義
// ========================================

// 建立 BMC11T001 物件，指定通訊介面為 Serial4
// BMC11T001 類別負責與 RFID 模組進行串列通訊
// BMduino UNO 的 Serial4 對應特定的硬體 UART 腳位
BMC11T001 BMC11(&Serial4);

// 儲存 UID 讀取回傳長度的變數
// 成功讀取 4 Bytes UID 時，此值應為 12（根據 BMC11T001 模組規格）
// 可用於判斷讀取是否成功
int nlens;

// UID 資料的接收緩衝區（最多 50 bytes）
// 用於儲存從 RFID 模組讀取到的原始 UID 資料
// 成功讀取後，uid_buf[0]~uid_buf[3] 儲存 UID 的 4 個位元組
uint8_t uid_buf[50] = {0};

// 字串變數用來存放卡號的 HEX 格式
// 例如："F0A1B2C3" 代表 UID 為 0xF0 0xA1 0xB2 0xC3
String uidByteStr = "";

// ========================================
// 函式前置宣告
// ========================================

// 初始化 RFID 讀卡模組
// 包含：延遲等待模組啟動、設定通訊波特率、啟動 ISO14443A 協定
void initRFID();

// 將 4 個 byte 的 UID 組合成一個 32-bit 整數
// 參數 d4（最高位元組）、d3、d2、d1（最低位元組）
unsigned long UUIDString(int d4, int d3, int d2, int d1);

// 檢查是否成功讀取到 RFID 卡片
// 回傳 true：有卡片且讀取成功，UID 資料已儲存於 uid_buf
// 回傳 false：無卡片或讀取失敗
boolean checkReadRFIDSuccess();

// 讀取 UID 並轉換為 10 位數字串（自動補零）
// 例如：305419896 轉為 "0305419896"
String readRFIDUUID();

// 將 UID 轉為 HEX 字串（例如："F0A1B2C3"）
// 適用於需要原始 HEX 格式的場景
String readRFIDUUIDString();

// 從 UID buffer 組成 HEX 字串，再轉為數字型態 UID（補零至 10 位）
// 與 readRFIDUUID() 功能類似，但使用不同的資料來源
String readRFIDUIDString();

// ========================================
// initRFID() 函式：初始化 RFID 讀卡模組
// ========================================
// 功能說明：
//   1. 延遲等待 RFID 模組完成上電啟動
//   2. 初始化 Serial4 硬體串列埠，設定通訊速率為 115200 bps
//   3. 啟動 ISO14443A 協定模式，準備讀取卡片
// 輸入參數：無
// 回傳值：無
// ========================================
void initRFID()
{
  // 延遲 1 秒鐘，等待 RFID 模組完成上電啟動
  // 確保模組內部電路穩定後再進行初始化
  // 若延遲不足可能導致初始化失敗
  delay(1000);
  
  // 初始化 Serial4 硬體串列埠，設定通訊速率為 115200 bps
  // BMC11T001 模組預設使用此波特率進行通訊
  // 若使用其他波特率需與模組設定一致
  BMC11.begin(115200);
  
  // 啟動 ISO14443A 協定模式
  // ISO14443A 是 MIFARE Classic、MIFARE Ultralight 等卡片使用的通訊協定
  // 此函式會發送特定指令讓模組進入卡片偵測模式
  BMC11.begin_ISO14443A();
}

// ========================================
// UUIDString() 函式：將 4 個 byte 的 UID 組合成一個 32-bit 整數
// ========================================
// 功能說明：
//   將 4 個獨立的位元組（Bytes）組合成一個 32 位元無號整數
//   公式：UID = d4 * 2^24 + d3 * 2^16 + d2 * 2^8 + d1
//   2^24 = 16777216
//   2^16 = 65536
//   2^8 = 256
// 參數說明：
//   d4：最高位元組（Byte 3）－ UID 的最高 8 位元
//   d3：次高位元組（Byte 2）－ UID 的次高 8 位元
//   d2：次低位元組（Byte 1）－ UID 的次低 8 位元
//   d1：最低位元組（Byte 0）－ UID 的最低 8 位元
// 回傳值：組合後的 32 位元無號整數（unsigned long）
// 使用範例：
//   UUIDString(0x12, 0x34, 0x56, 0x78) 回傳 0x12345678
// ========================================
unsigned long UUIDString(int d4, int d3, int d2, int d1)
{
  // 利用位元加權方式組合 UID 整數值
  // 將 d4 乘以 2^24（16777216）
  // 加上 d3 乘以 2^16（65536）
  // 加上 d2 乘以 2^8（256）
  // 加上 d1
  unsigned long tmp = 0;
  tmp = d4 * (unsigned long)16777216 + d3 * (unsigned long)65536 + d2 * (unsigned long)256 + (unsigned long)d1;
  return tmp;
}

// ========================================
// checkReadRFIDSuccess() 函式：檢查是否成功讀取到 RFID 卡片
// ========================================
// 功能說明：
//   從 RFID 模組讀取 ISO14443A 卡片的 UID
//   根據回傳長度判斷是否成功讀取到卡片
// 輸入參數：無
// 回傳值：
//   true：成功讀取到卡片，UID 資料已儲存於 uid_buf 陣列
//   false：無卡片靠近或讀取失敗
// 注意事項：
//   當此函式回傳 true 後，uid_buf 才包含有效的 UID 資料
//   若回傳 false，uid_buf 內容為上次讀取的殘留資料，不應使用
// ========================================
boolean checkReadRFIDSuccess() {
  // 從 RFID 模組讀取 ISO14443A 卡片的 UID
  // 參數 uid_buf 為儲存 UID 資料的緩衝區
  // 回傳值 nlens 為讀取到的資料長度（bytes）
  nlens = BMC11.getUID_ISO14443A(uid_buf);
  
  // 檢查回傳長度是否為 12
  // 根據 BMC11T001 模組規格，成功讀取 4 Bytes UID 時會回傳 12
  // 這 12 個位元組包含 UID 資料與額外的格式資訊
  if (nlens == 12) {
    return true;   // 讀取成功，有卡片靠近
  } else {
    return false;  // 讀取失敗或無卡片
  }
}

// ========================================
// readRFIDUUID() 函式：讀取 RFID UID，轉換為整數字串，並補足至 10 位數
// ========================================
// 功能說明：
//   1. 從 uid_buf 中提取 UID 的四個位元組
//   2. 組合成 32 位元整數
//   3. 轉換為字串格式
//   4. 若長度不足 10 位數，自動在前面補零
// 輸入參數：無（使用全域變數 uid_buf）
// 回傳值：10 位數的 UID 字串（例如："0305419896"）
// 使用範例：
//   UID = 0x12 0x34 0x56 0x78 -> 305419896 -> "0305419896"
// 注意事項：
//   呼叫此函式前應先呼叫 checkReadRFIDSuccess() 確認有卡片
// ========================================
String readRFIDUUID() {
  String tmp = "";           // 暫存結果字串
  unsigned long ttmp;        // 暫存 UID 整數值
  
  // 透過 UUIDString() 將 UID 四個 byte 組合成整數
  // 注意：uid_buf 陣列中的索引對應關係
  // uid_buf[3]：最高位元組（Byte 3）
  // uid_buf[2]：次高位元組（Byte 2）
  // uid_buf[1]：次低位元組（Byte 1）
  // uid_buf[0]：最低位元組（Byte 0）
  ttmp = UUIDString(
    uid_buf[3],   // 最高位元組
    uid_buf[2],   // 次高位元組
    uid_buf[1],   // 次低位元組
    uid_buf[0]    // 最低位元組
  );
  
  // 將整數轉換為字串格式
  tmp = String(ttmp);
  
  // 輸出除錯訊息至序列監控視窗
  // 便於開發者確認 UID 轉換結果
  Serial.print("tmp is :("); 
  Serial.print(tmp); 
  Serial.println(")");
  
  // 取得字串長度
  int len = tmp.length();
  Serial.print("Len is :("); 
  Serial.print(len); 
  Serial.println(")");
  
  // 檢查長度是否小於 10 位數
  if (len < 10) {
    // 若長度不足 10 位，使用 genstr() 函式在前面補零
    // genstr('0', 10 - len) 會產生由指定數量 '0' 組成的字串
    // 例如：10 - len = 3 時，產生 "000"
    return genstr('0', 10 - len) + tmp;
  } else {
    return tmp;
  }
  return tmp;  // 備援回傳（理論上不會執行到此）
}

// ========================================
// readRFIDUUIDString() 函式：將 UID（4 Bytes）轉為 HEX 字串
// ========================================
// 功能說明：
//   將 UID 的四個位元組轉換為十六進制字串表示法
//   高位元組在前，低位元組在後
// 輸入參數：無（使用全域變數 uid_buf）
// 回傳值：HEX 格式的 UID 字串（例如："F0A1B2C3"）
// 使用範例：
//   UID = 0xF0 0xA1 0xB2 0xC3 -> "F0A1B2C3"
// 應用場景：
//   需要將 UID 以 HEX 格式儲存或傳輸時使用
//   例如：雲端資料庫儲存、MQTT 訊息發布
// ========================================
String readRFIDUUIDString()
{
  String uidString = "";  // 暫存 HEX 字串結果
  
  // 從 uid_buf 的第 3~0 個 byte 轉為 HEX（高位在前）
  // 迴圈從 i=3 遞減到 i=0，確保最高位元組先處理
  for (int i = 3; i >= 0; i--) {
    // print2HEX() 函式將單一位元組轉換為兩位 HEX 字串
    // 例如：0x0F 轉換為 "0F"
    // 例如：0xAB 轉換為 "AB"
    uidString += print2HEX((int)uid_buf[i]);
  }
  
  // 將整個 HEX 字串轉為大寫，確保格式一致性
  // 例如："f0a1b2c3" 轉為 "F0A1B2C3"
  uidString.toUpperCase();
  
  return uidString;
}

// ========================================
// readRFIDUIDString() 函式：從 UID buffer 的 Byte 內容組成 HEX 字串，
// 再轉換為常見數字型態 UID（自動補零至 10 位）
// ========================================
// 功能說明：
//   此函式提供另一種 UID 解析方式，從 uid_buf 索引 4~11 的位置
//   讀取以 ASCII 字元形式儲存的 UID HEX 表示，再轉換為 10 位數字串
// 輸入參數：無（使用全域變數 uid_buf）
// 回傳值：10 位數的 UID 字串（例如："0305419896"）
// 與 readRFIDUUID() 的差異：
//   - readRFIDUUID()：直接使用 uid_buf[0]~uid_buf[3] 的二進位值
//   - readRFIDUIDString()：使用 uid_buf[4]~uid_buf[11] 的 ASCII 字元
// 使用時機：
//   當模組以 ASCII 字串形式回傳 UID 時使用此函式
// ========================================
String readRFIDUIDString()
{
  String tmp = "";               // 回傳最終結果
  unsigned long ttmp;            // 暫存整數型 UID
  String uidString = "";         // 暫存 HEX 字串 UID
  int d1, d2, d3, d4;            // 四個 byte 對應的十進位整數值
  String s1, s2, s3, s4;         // 各 byte 的 HEX 字串（兩位）
  
  // 將 uid_buf 的第 4~11 byte（共 8 個）轉成字串
  // 這些位元組以 ASCII 字元形式儲存 UID 的 HEX 表示
  // 例如：uid_buf[4]='1', uid_buf[5]='2', uid_buf[6]='3', ...
  for (int i = 4; i < 12; i++) {
    uidString += String(char(uid_buf[i]));
  }
  
  // 將整個字串轉為大寫，確保格式一致性
  uidString.toUpperCase();
  
  // 從字串中取出每兩位作為一個 byte 的 HEX 字串
  // 例如：uidString = "12345678"
  // substring(0, 2) 取索引 0~1 得到 "12"
  // substring(2, 4) 取索引 2~3 得到 "34"
  // substring(4, 6) 取索引 4~5 得到 "56"
  // substring(6, 8) 取索引 6~7 得到 "78"
  s1 = uidString.substring(0, 2);
  s2 = uidString.substring(2, 4);
  s3 = uidString.substring(4, 6);
  s4 = uidString.substring(6, 8);
  
  // 將 HEX 字串轉為十進位整數
  // strtoul() 函式：將字串轉換為無號長整數（string to unsigned long）
  // 參數說明：
  //   s1.c_str()：將 String 轉為 C 風格字串（以 '\0' 結尾）
  //   NULL：不儲存剩餘字串的指標位置
  //   16：表示輸入字串為 16 進位格式
  d1 = (int)strtoul(s1.c_str(), NULL, 16);
  d2 = (int)strtoul(s2.c_str(), NULL, 16);
  d3 = (int)strtoul(s3.c_str(), NULL, 16);
  d4 = (int)strtoul(s4.c_str(), NULL, 16);
  
  // 將四個 byte 組合成一個長整數
  // 注意參數順序：d4（最高位元組）、d3、d2、d1（最低位元組）
  ttmp = UUIDString(d4, d3, d2, d1);
  
  // 將整數轉換為字串
  tmp = String(ttmp);
  
  // 輸出除錯訊息至序列監控視窗
  Serial.print("tmp is :("); 
  Serial.print(tmp); 
  Serial.println(")");
  
  // 取得字串長度
  int len = tmp.length();
  Serial.print("Len is :("); 
  Serial.print(len); 
  Serial.println(")");
  
  // 若長度不足 10 位數，自動補 0
  if (len < 10) {
    return genstr('0', 10 - len) + tmp;
  } else {
    return tmp;
  }
  return tmp;  // 備援回傳（理論上不會執行到此）
}