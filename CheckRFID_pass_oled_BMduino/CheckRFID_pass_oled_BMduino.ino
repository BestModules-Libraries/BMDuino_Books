/*
 * 程式名稱：註冊RFID裝置之雲端門禁裝置 (regRFIDDevice_Clouding_BMduino)
 * 程式語言：Arduino C/C++
 * 
 * ========================================
 * 程式功能完整解說
 * ========================================
 * 
 * 【系統概述】
 *   本程式為一個基於 RFID 技術的雲端門禁系統，主要功能是讀取 RFID 卡片，
 *   將卡號透過 Wi-Fi 上傳至雲端伺服器進行門禁權限驗證或記錄。
 * 
 * 【硬體組成】
 *   1. BMduino 相容開發板（Arduino 相容開發板）
 *   2. RFID 讀卡模組（BMC11T001）- 用於讀取 RFID 卡片 UID
 *   3. OLED 顯示模組（0.96 吋，BMD31M090）- 顯示系統狀態與卡號
 *   4. 繼電器模組（BMP75M131）- 用於控制門鎖或其他裝置
 *   5. Wi-Fi 模組 - 用於連接網路並傳送資料至雲端
 * 
 * 【運作流程】
 *   1. 初始化階段（setup 函式）：
 *      a. 呼叫 initAll() 初始化序列埠、繼電器、OLED、Wi-Fi 及 RFID 模組
 *      b. 透過 GetSSID()、GetIP()、GetMAC() 取得網路連線資訊
 *      c. 將 MAC 位址、IP 位址等資訊顯示於序列監控視窗
 *      d. 在 OLED 上顯示廠商 LOGO 2 秒後清除
 *      e. 將 MAC 位址與 IP 位址顯示在 OLED 螢幕上
 * 
 *   2. 主迴圈階段（loop 函式）：
 *      a. 呼叫 checkReadRFIDSuccess() 檢查是否有 RFID 卡片靠近
 *      b. 若偵測到卡片，呼叫 readRFIDUIDString() 讀取卡片 UID
 *      c. 將讀取到的卡號顯示於序列監控視窗與 OLED 螢幕
 *      d. 檢查 Wi-Fi 連線狀態（Wifi.getStatus()）
 *      e. 若 Wi-Fi 連線正常，呼叫 SendtoClouding() 將卡號傳送至雲端
 *      f. 延遲 2 秒後重複執行，避免過度頻繁讀取
 * 
 * 【資料傳輸】
 *   - 使用 HTTP GET 方式將卡號資料傳送至雲端伺服器
 *   - 雲端傳送功能由 clouding.h 函式庫中的 SendtoClouding() 提供
 * 
 * 【顯示介面】
 *   - OLED 螢幕分為多行顯示：
 *     第 0 行：顯示 MAC 位址
 *     第 2 行：顯示 IP 位址
 *     第 4 行：顯示讀取到的 RFID 卡號
 *     第 6 行：顯示系統訊息或傳送結果
 * 
 * 【開發環境】
 *   - 開發板：BMduino 或 Arduino 相容開發板
 *   - 程式語言：Arduino C/C++
 *   - 相依函式庫：RelayLib、TCP.h、OledLib.h、RFIDLib.h、clouding.h
 * 
 * 【注意事項】
 *   - 使用前需確認所有模組的 I2C 位址無衝突
 *   - Wi-Fi 連線設定需在 RelayLib 或 TCP.h 中預先設定 SSID 與密碼
 *   - 雲端伺服器網址需在 clouding.h 中正確設定
 *   - 序列監控視窗鮑率需設定為 9600 bps
 * 
 * 【版本資訊】
 *   最後修改日期：2026年3月26日
 *   作者：regRFIDDevice_Clouding_BMduino
 */

// 註冊RFID裝置之雲端門禁裝置  regRFIDDevice_Clouding_BMduino

// ========================================
// 全域變數定義
// ========================================
String SSIDData ;           // 儲存所連接的 Wi-Fi AP（無線基地台）的 SSID 名稱
String IPData ;             // 儲存開發板取得的 IP 位址字串（例如 "192.168.1.100"）
String MacData ;            // 儲存開發板的 MAC 位址字串（例如 "AA:BB:CC:DD:EE:FF"）
//String uidStr ;             // 儲存讀取到的 RFID 卡片 UID 字串

// ========================================


#include <String.h>         // 引入字串處理函式庫，用於字串的儲存與操作
#include "RelayLib.h"       // 引入繼電器控制函式庫，包含繼電器初始化、Wi-Fi 初始化及 MAC 位址取得等功能
#include "TCP.h"            // 引入 TCP 通訊函式庫，包含網路連線相關的函式
#include "OledLib.h"        // 引入 OLED 顯示模組自訂函式庫，提供螢幕顯示相關函式
#include "RFIDLib.h"        // 引入 RFID 讀卡模組函式庫，用於 BMC11T001 RFID 讀寫模組的通訊控制
#include "clouding.h"       // 引入雲端通訊函式庫，提供 HTTP GET 方式將資料傳送至雲端伺服器的功能

// ========================================
// 函式前置宣告
// ========================================
void initSensor();          // 初始化所有感測模組（RFID、OLED、繼電器、Wi-Fi）
void initAll();             // 初始化整體系統（序列埠、感測器、LED 狀態等）
void PrintMAConOLED(String ss);    // 在 OLED 螢幕上顯示 MAC 位址
void PrintSSIDonOLED(String ss);   // 在 OLED 螢幕上顯示 AP SSID 名稱
void PrintIPonOLED(String ss);     // 在 OLED 螢幕上顯示取得的 IP 位址
void PrintCardonOLED(String ss);   // 在 OLED 螢幕上顯示讀取到的 RFID 卡號
void PrintmsgonOLED(String ss);    // 在 OLED 螢幕上顯示系統訊息或結果資訊

// ========================================
// setup() 函式：Arduino 啟動時只執行一次
// ========================================
void setup() 
{
  initAll();          // 初始化整體系統（序列埠、繼電器、OLED、Wi-Fi、RFID 等模組）

  Serial.println("");                 // 輸出空行，讓序列監控視窗的顯示更美觀
  Serial.println("---wifi access point----"); // 輸出分隔線，標示即將顯示 Wi-Fi 資訊

  SSIDData = GetSSID();               // 呼叫 GetSSID() 函式取得目前連線的 AP SSID 字串
  Serial.println(SSIDData);           // 將 SSID 輸出至序列監控視窗，供除錯檢視

  IPData = GetIP();                   // 呼叫 GetIP() 函式取得開發板取得的閘道器 IP 位址字串
  Serial.println("---Show IP Address----"); // 輸出分隔線，標示即將顯示 IP 位址
  Serial.println(IPData);             // 將 IP 位址輸出至序列監控視窗

  MacData = GetMAC();                 // 呼叫 GetMAC() 函式取得開發板的 MAC 位址字串
  Serial.println("---MAC Address----");   // 輸出分隔線，標示即將顯示 MAC 位址
  Serial.println(MacData);            // 將 MAC 位址輸出至序列監控視窗

  initAll();                          // 再次初始化整體系統，確保所有模組穩定運作

  drawPicture(0, 0, BestModule_LOGO, 128, 64); // 在 OLED 螢幕上顯示廠商 LOGO 點陣圖
                                              // 參數說明：(x座標, y座標, 圖案陣列, 寬度, 高度)
  delay(2000);                        // 延遲 2 秒鐘，讓 LOGO 有足夠時間顯示
  clearScreen();                      // 清除 OLED 螢幕上的所有內容

  PrintMAConOLED(MacData);            // 在 OLED 螢幕上顯示 MAC 位址（第 0 行）
  //PrintSSIDonOLED(SSIDData);        // （註解狀態）可選擇性開啟，在 OLED 上顯示 AP SSID
  PrintIPonOLED(IPData);              // 在 OLED 螢幕上顯示取得的 IP 位址（第 2 行）
}

// ========================================
// loop() 函式：Arduino 啟動後會重複執行此區塊的程式碼
// ========================================
void loop()
{
  // 檢查是否有 RFID 卡片靠近並成功讀取
  // checkReadRFIDSuccess() 回傳 true 表示偵測到卡片且讀取成功
  if (checkReadRFIDSuccess())
  {
    uidStr = readRFIDUIDString();     // 讀取 RFID 卡片的 UID 並轉換為字串格式

    // 將讀取到的卡號輸出至序列監控視窗，供除錯檢視
    Serial.print("read RFID Card Number is :(");
    Serial.print(uidStr);
    Serial.print(")\n");

    PrintCardonOLED(uidStr);          // 在 OLED 螢幕上顯示讀取到的卡號（第 4 行）

    // 檢查 Wi-Fi 連線狀態是否正常
    // Wifi.getStatus() 回傳 true 表示 Wi-Fi 已連線且運作正常
    if (Wifi.getStatus())
    {
      Serial.println("WIFI OK");      // 輸出 Wi-Fi 連線正常訊息至序列監控視窗
      SendtoClouding();               // 呼叫 SendtoClouding() 函式，將感測資料（卡號）透過 HTTP 傳送至雲端伺服器
    }
    else
    {
      // 若 Wi-Fi 連線異常，可在此處加入錯誤處理邏輯（例如顯示錯誤訊息）
      // 本程式目前未實作此部分的處理
    }
  }

  // 延遲 2 秒鐘（2000 毫秒）再進行下一次的讀卡動作
  // 避免因程式循環過快導致重複讀取同一張卡片，或造成雲端伺服器過度負載
  delay(2000);
}

// ========================================
// initSensor() 函式：初始化所有感測模組
// ========================================
void initSensor()
{
  initRelay();      // 初始化繼電器模組（BMP75M131），透過 I2C 通訊協定控制
                    // 繼電器可用於控制門鎖、電磁開關等裝置
  
  initOled();       // 初始化 OLED 12864 顯示模組（0.96 吋，BMD31M090）
                    // 設定 I2C 通訊參數，準備進行螢幕顯示
  
  initWiFi();       // 初始化 Wi-Fi 連線模組
                    // 此函式應包含 SSID 與密碼的設定，以及連線至無線網路
  
  initRFID();       // 初始化 RFID 讀卡模組（BMC11T001）
                    // 設定 I2C 通訊，準備讀取 RFID 卡片 UID
}

// ========================================
// initAll() 函式：初始化整體系統
// ========================================
void initAll()
{
  Serial.begin(9600);               // 啟動序列埠通訊，設定傳輸速率為 9600 bps
                                    // 此速率需與 Arduino IDE 序列監控視窗設定的鮑率一致
  digitalWrite(LED, LOW);           // 將內建 LED 腳位設定為 LOW 電位（熄滅）
                                    // 作為開機狀態指示，可根據實際需求修改為閃爍或其他模式
  initSensor();                     // 呼叫 initSensor() 函式，初始化所有感測器模組
}

// ========================================
// PrintMAConOLED() 函式：在 OLED 螢幕上顯示 MAC 位址
// 參數：ss - 要顯示的 MAC 位址字串
// ========================================
void PrintMAConOLED(String ss)
{
  printText(0, 0, "              "); // 先清除第 0 行的內容（填入空白字元覆蓋）
  printText(0, 0, ss);              // 在第 0 行顯示 MAC 位址字串
  Serial.print("MAC on OLED:(");    // 輸出除錯訊息至序列監控視窗
  Serial.print(ss);
  Serial.print(")\n");
}

// ========================================
// PrintSSIDonOLED() 函式：在 OLED 螢幕上顯示 AP SSID 名稱
// 參數：ss - 要顯示的 SSID 字串
// ========================================
void PrintSSIDonOLED(String ss)
{
  printText(0, 4, "              "); // 先清除第 4 行的內容（填入空白字元覆蓋）
  printText(0, 4, ss);              // 在第 4 行顯示 SSID 字串
  Serial.print("SSID on OLED:(");   // 輸出除錯訊息至序列監控視窗
  Serial.print(ss);
  Serial.print(")\n");
}

// ========================================
// PrintIPonOLED() 函式：在 OLED 螢幕上顯示取得的 IP 位址
// 參數：ss - 要顯示的 IP 位址字串
// ========================================
void PrintIPonOLED(String ss)
{
  printText(0, 2, "              "); // 先清除第 2 行的內容（填入空白字元覆蓋）
  printText(0, 2, ss);              // 在第 2 行顯示 IP 位址字串
  Serial.print("IP on OLED:(");     // 輸出除錯訊息至序列監控視窗
  Serial.print(ss);
  Serial.print(")\n");
}

// ========================================
// PrintCardonOLED() 函式：在 OLED 螢幕上顯示讀取到的 RFID 卡號
// 參數：ss - 要顯示的卡號字串
// ========================================
void PrintCardonOLED(String ss)
{
  printText(0, 4, "              "); // 先清除第 4 行的內容（填入空白字元覆蓋）
  printText(0, 4, ss);              // 在第 4 行顯示卡號字串
  Serial.print("RFID Number on OLED:("); // 輸出除錯訊息至序列監控視窗
  Serial.print(ss);
  Serial.print(")\n");
}

// ========================================
// PrintmsgonOLED() 函式：在 OLED 螢幕上顯示系統訊息或結果資訊
// 參數：ss - 要顯示的訊息字串
// ========================================
void PrintmsgonOLED(String ss)
{
  printText(0, 6, "              "); // 先清除第 6 行的內容（填入空白字元覆蓋）
  printText(0, 6, ss);              // 在第 6 行顯示訊息字串
  Serial.print("Message on OLED:("); // 輸出除錯訊息至序列監控視窗
  Serial.print(ss);
  Serial.print(")\n");
}