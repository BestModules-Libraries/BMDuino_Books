/*
 * 程式名稱：門禁裝置讀取卡號查詢雲端授權開關門程式 (CheckRFID_forpass_oled_BMduino)
 * 程式語言：Arduino C/C++
 * 
 * ========================================
 * 程式整體運作解說
 * ========================================
 * 
 * 【1. 系統概述】
 * 本程式設計用於一個基於 Arduino 的門禁控制系統，主要功能為：
 * - 透過 RFID 模組讀取使用者的卡片或鑰匙圈上的 UID（唯一識別碼）
 * - 將讀取到的卡號透過 Wi-Fi 網路，以 HTTP GET 方式傳送至雲端伺服器進行授權驗證
 * - 依據雲端伺服器回傳的結果（此部分在 clouding.h 中實作，本程式未顯示細節），
 *   控制繼電器來開啟或關閉門鎖（或其它裝置）
 * - 同時在 OLED 螢幕上顯示相關資訊，如 MAC 位址、IP 位址、讀取的卡號及系統訊息
 * 
 * 【2. 硬體組成與對應函式庫】
 * - BMduino 開發板（相容於 Arduino）作為控制核心
 * - BMC11T001 RFID 讀寫模組（I2C 介面）：用於讀取卡片 UID
 * - BMP75M131 繼電器模組（I2C 介面）：用於控制門鎖的開關
 * - BMD31M090 OLED 12864 顯示模組（0.96 吋，I2C 介面）：用於顯示文字與圖案
 * - Wi-Fi 模組（內建或外接）：用於連接網路與雲端伺服器通訊
 * 
 * 【3. 主要流程】
 * (1) 啟動與初始化（setup 函式）：
 *     - 初始化序列埠、感測器模組（RFID、繼電器、OLED、Wi-Fi）
 *     - 連線至預設的 Wi-Fi 熱點（SSID 與密碼預設於 TCP.h 中）
 *     - 取得並顯示 MAC 位址、SSID、IP 位址於序列監控視窗與 OLED 螢幕
 *     - 顯示廠商 Logo 2 秒後清除螢幕
 * 
 * (2) 主迴圈（loop 函式）：
 *     - 持續檢查是否有 RFID 卡片靠近
 *     - 若讀取到卡片，則取得其 UID 並顯示於 OLED 與序列埠
 *     - 檢查 Wi-Fi 連線狀態，若正常則呼叫 SendtoClouding() 將卡號傳送至雲端
 *       （雲端伺服器會回傳是否允許開門的指令，由 clouding.h 內部處理）
 *     - 若 Wi-Fi 連線異常，則不進行雲端查詢（此版本未加入錯誤重試機制）
 *     - 延遲 2 秒以避免重複讀取或過度頻繁的網路請求
 * 
 * 【4. 關鍵函式說明】
 * - initAll()：整體初始化，包含序列埠、LED 狀態、感測器模組
 * - initSensor()：初始化 RFID、繼電器、OLED、Wi-Fi 等硬體模組
 * - INITWIFI() / GetWiFiInformation() / ShowWiFiInformation()：
 *   負責 Wi-Fi 連線、取得 SSID/IP/MAC 等資訊，並顯示於序列埠
 * - Print...onOLED() 系列函式：在不同行數顯示 MAC、IP、卡號等資訊於 OLED
 * - checkReadRFIDSuccess() / readRFIDUIDString()：來自 RFIDLib.h，
 *   偵測與讀取卡片 UID
 * - SendtoClouding()：來自 clouding.h，負責將卡號以 HTTP GET 送出至雲端，
 *   並依據回應控制繼電器（開關門鎖）
 * 
 * 【5. 外部依賴函式庫說明】
 * - RelayLib.h：提供 initRelay()、控制繼電器的函式（如開門/關門）
 * - TCP.h：提供 Wi-Fi 連線相關函式（initWiFi()、GetSSID()、GetIP()、GetMAC()）
 * - OledLib.h：提供 OLED 顯示相關函式（initOled()、printText()、clearScreen()、
 *   drawPicture() 等）
 * - RFIDLib.h：提供 RFID 模組初始化與讀取 UID 的函式
 * - clouding.h：提供雲端通訊函式 SendtoClouding()，內部實作 HTTP GET 請求
 *   並根據回應控制繼電器動作
 * 
 * 【6. 注意事項】
 * - 本程式假設所有周邊模組均透過 I2C 介面連接，且位址設定與函式庫內部預設相符
 * - Wi-Fi 的 SSID 與密碼需預先在 TCP.h 中定義（例如 #define WIFI_SSID "your_SSID"）
 * - 雲端伺服器的 URL 與參數設定需在 clouding.h 中完成
 * - 若 Wi-Fi 連線中斷，本程式目前不會自動重新連線，也無錯誤訊息顯示於 OLED
 * - 延遲 2 秒可能影響即時性，可視需求調整，但需避免雲端伺服器過度負載
 * - 本程式為展示基本功能，實際部署時建議加入更多錯誤處理與重試機制
 * 
 * 【7. 版本與修改記錄】
 * - 原始版本：門禁裝置讀取卡號查詢雲端授權開關門程式 (CheckRFID_forpass_oled_BMduino)
 * - 本次修改：加入詳細繁體中文註解與整體運作說明
 */

// ========================================
// 全域變數定義
// ========================================
String SSIDData ;           // 儲存所連接的 Wi-Fi AP（無線基地台）的 SSID 名稱
String IPData ;             // 儲存開發板取得的 IP 位址字串（例如 "192.168.1.100"）
String MacData ;            // 儲存開發板的 MAC 位址字串（例如 "AA:BB:CC:DD:EE:FF"）
// String uidStr ;           // 儲存讀取到的 RFID 卡片 UID 字串（實際使用於 RFIDLib.h 中定義）

// ========================================
// 引入所需函式庫
// ========================================
#include <String.h>         // 引入字串處理函式庫，用於字串的儲存與操作
#include "RelayLib.h"       // 引入繼電器控制函式庫，包含繼電器初始化、Wi-Fi 初始化及 MAC 位址取得等功能
#include "TCP.h"            // 引入 TCP 通訊函式庫，包含網路連線相關的函式
#include "OledLib.h"        // 引入 OLED 顯示模組自訂函式庫，提供螢幕顯示相關函式
#include "RFIDLib.h"        // 引入 RFID 讀卡模組函式庫，用於 BMC11T001 RFID 讀寫模組的通訊控制
#include "clouding.h"       // 引入雲端通訊函式庫，提供 HTTP GET 方式將資料傳送至雲端伺服器的功能
#include "16keyLib.h"       // 引入雲端通訊函式庫，提供 HTTP GET 方式將資料傳送至雲端伺服器的功能

// ========================================
// 函式前置宣告
// ========================================
void initSensor();          // 初始化所有感測模組（RFID、OLED、繼電器、Wi-Fi）
void initAll();             // 初始化整體系統（序列埠、感測器、LED 狀態等）
void GetWiFiInformation();  // 取得 WiFi 資訊
void INITWIFI();            // 初始化 Wi-Fi 網路連線
void ShowWiFiInformation(); // 顯示 Wi-Fi 連線資訊（MAC、SSID、IP）
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
  
  // INITWIFI() 會執行以下操作：
  //   - 呼叫 GetWiFiInformation() 取得目前 WiFi 資訊
  //   - 呼叫 initWiFi() 連線至預先設定的無線網路
  //   - 呼叫 ShowWiFiInformation() 顯示連線資訊（MAC、SSID、IP）
  INITWIFI();          // 初始化 WiFi 網路
                       // 此函式應包含 SSID 與密碼的設定，以及連線至無線網路
  init16key();      // 初始化 BMK52T016 按鍵模組
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
// INITWIFI() 函式：初始化 WiFi 無線網路連線
// ========================================
/**
 * 函式名稱：INITWIFI()
 * 功能：初始化 WiFi 無線網路連線
 * 說明：
 *   1. 呼叫 GetWiFiInformation() 取得目前 WiFi 資訊
 *   2. 呼叫 initWiFi() 進行 WiFi 連線，連線資訊（SSID、密碼）應預先設定於 TCP.h 中
 *   3. 連線完成後呼叫 ShowWiFiInformation() 顯示網路相關資訊
 * 
 * 注意事項：
 *   - 若連線失敗，initWiFi() 應包含重試機制或錯誤處理
 *   - Wi-Fi 連線成功後方可進行後續的雲端通訊
 */
void INITWIFI()      // 初始化 WiFi 網路
{
  GetWiFiInformation();  // 取得 WiFi 資訊（SSID、IP、MAC）
  initWiFi();             // 連線到預先設定的 WiFi 熱點
                          // 連線資訊（SSID、密碼）應於 TCP.h 中定義
  
  ShowWiFiInformation();  // 顯示 MAC / SSID / IP 等連線資訊
}

// ========================================
// GetWiFiInformation() 函式：取得 WiFi 資訊
// ========================================
/**
 * 函式名稱：GetWiFiInformation()
 * 功能：取得目前 WiFi 連線相關資訊（SSID、IP、MAC）
 * 說明：
 *   此函式會取得裝置連線的 SSID、被分配的 IP 位址、以及裝置的 MAC 位址
 *   並將這些資訊儲存到全域變數中，同時也輸出到序列埠監控視窗
 * 
 * 輸出資訊：
 *   - SSID：目前連線的無線網路名稱
 *   - IP Address：設備在區域網路中被分配的 IP 位址
 *   - MAC Address：設備的實體位址，作為設備唯一識別碼
 */
void GetWiFiInformation()  // 取得 WiFi 資訊
{
  // 步驟 1：輸出分隔線與標題，便於在序列監控視窗中閱讀
  Serial.println("");                 // 輸出空行，讓序列監控視窗的顯示更美觀
  Serial.println("---wifi access point----"); // 輸出分隔線，標示即將顯示 Wi-Fi 資訊

  // 步驟 2：取得並顯示目前連線的 Wi-Fi AP 名稱（SSID）
  SSIDData = GetSSID();               // 呼叫 GetSSID() 函式取得目前連線的 AP SSID 字串
  Serial.println(SSIDData);           // 將 SSID 輸出至序列監控視窗，供除錯檢視

  // 步驟 3：取得並顯示開發板的 IP 位址
  IPData = GetIP();                   // 呼叫 GetIP() 函式取得開發板取得的閘道器 IP 位址字串
  Serial.println("---Show IP Address----"); // 輸出分隔線，標示即將顯示 IP 位址
  Serial.println(IPData);             // 將 IP 位址輸出至序列監控視窗

  // 步驟 4：取得並顯示開發板的 MAC 位址
  MacData = GetMAC();                 // 呼叫 GetMAC() 函式取得開發板的 MAC 位址字串
  Serial.println("---MAC Address----");   // 輸出分隔線，標示即將顯示 MAC 位址
  Serial.println(MacData);            // 將 MAC 位址輸出至序列監控視窗
}

// ========================================
// ShowWiFiInformation() 函式：顯示 WiFi 連線資訊
// ========================================
/**
 * 函式名稱：ShowWiFiInformation()
 * 功能：顯示目前 WiFi 連線相關資訊（MAC、SSID、IP）
 * 說明：
 *   此函式會取得裝置的 MAC 位址、連線的 SSID、以及被分配的 IP 位址
 *   並將這些資訊儲存到全域變數中，同時也輸出到序列埠監控視窗
 *   這些資訊後續也會用於 OLED 螢幕顯示或雲端通訊
 * 
 * 輸出資訊：
 *   - MAC Address：設備的實體位址，作為設備唯一識別碼
 *   - SSID：目前連線的無線網路名稱
 *   - IP Address：設備在區域網路中被分配的 IP 位址
 */
void ShowWiFiInformation()  // 顯示 WiFi 資訊
{
  // 步驟 1：取得並儲存 MAC Address
  // GetMAC() 會從 Wi-Fi 模組讀取設備的實體位址
  // MAC 位址格式通常為 "AA:BB:CC:DD:EE:FF" 或 "AABBCCDDEEFF"
  MacData = GetMAC();   // 呼叫自訂函式 GetMAC()，取得目前裝置的 MAC Address
                        // 並將結果存入全域變數 MacData

  Serial.println("---MAC Address----");  // 在序列埠輸出標題文字
  Serial.println(MacData);               // 印出裝置的 MAC Address
  Serial.println("");                    // 空一行，使顯示格式更清楚

  // 步驟 2：取得並儲存 WiFi SSID
  // GetSSID() 會取得目前連線的無線網路名稱（Service Set Identifier）
  Serial.println("---wifi access point----"); // 顯示目前使用的 WiFi AP 資訊標題
  SSIDData = GetSSID(); // 呼叫 GetSSID() 取得目前連線的 WiFi 熱點名稱 (SSID)
                        // 並儲存回全域變數 SSIDData

  Serial.println(SSIDData);  // 印出目前連線的 WiFi 熱點名稱 (SSID)

  // 步驟 3：取得並儲存 IP Address
  // GetIP() 會取得設備在區域網路中被分配的 IP 位址
  // IP 位址格式為 "192.168.x.x" 或 "10.x.x.x" 等
  Serial.println("---Show IP Address----"); // 顯示 IP 位址標題
  IPData = GetIP();     // 呼叫自訂函式 GetIP() 取得目前 WiFi 分配到的 IP 位址
                        // 並儲存回全域變數 IPData

  Serial.println(IPData);  // 在序列埠輸出 IP Address，例如：192.168.1.105
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