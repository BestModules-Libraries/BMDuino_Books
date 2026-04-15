/*
 * =============================================================================
 * 檔案名稱: invalidRFID_inrfidlist_BMduino.ino
 * 程式語言: Arduino C/C++
 * 檔案類型: 主程式檔（Sketch File）
 * 功能說明: 門禁裝置讀取卡號傳送雲端失效程式（卡片權限終止）
 * 
 * =============================================================================
 * 程式運作流程完整解說
 * =============================================================================
 * 
 * 【檔案概述】
 *   本程式為雲端門禁系統的 BMduino 裝置端程式，負責讀取 RFID 卡片，
 *   並透過 HTTP GET 請求將卡片號碼傳送至雲端伺服器進行卡片權限終止（權限終止）作業。
 *   當感應到已授權的卡片時，系統會將該卡片的啟用狀態從「已啟用」改為「已失效」，
 *   使其無法再開啟門鎖。
 * 
 * 【主要功能】
 *   1. 初始化系統（序列埠、LED、Wi-Fi、RFID 模組）
 *   2. 持續偵測是否有 RFID 卡片靠近
 *   3. 讀取到卡片後，取得卡片的 UID（唯一識別碼）
 *   4. 檢查 Wi-Fi 連線狀態，若未連線則重新連線
 *   5. 透過 HTTP GET 將 MAC 位址與卡片號碼傳送至雲端 API
 *   6. 接收雲端回傳結果（JSON 格式）
 *   7. 等待 2 秒後繼續下一次讀卡循環
 * 
 * 【與啟用程式的差異說明】
 *   本程式與啟用程式（activateRFID_inrfidlist_BMduino.ino）的差異在於：
 *   - 啟用程式：將卡片的 activate 狀態設為 1（啟用），若卡片不存在則自動新增
 *   - 停用程式：將卡片的 activate 狀態設為 2（停用），僅對已存在的卡片生效
 * 
 * 【通訊協定說明】
 *   本系統使用 HTTP GET 方法將資料傳送至雲端伺服器：
 *     - 伺服器位址：http://iot.arduino.org.tw:8888
 *     - API 路徑：/bmduino/rfid/invalidrfid.php
 *     - 傳送參數：MAC（設備 MAC 位址）、KEY（RFID 卡號）
 * 
 * 【API 請求範例】
 *   http://iot.arduino.org.tw:8888/bmduino/rfid/invalidrfid.php?MAC=483FDACA32A3&KEY=1111222233
 * 
 * 【雲端 API 回傳結果說明 (JSON 格式)】
 *   1. 裝置不存在或未啟用：
 *      {"Device": "B8D61A68DAFC", "Card": "1122334455", "Result": "Device Not Existed"}
 *      說明：此設備尚未在雲端系統中註冊或已被停用
 * 
 *   2. 卡片已停用（從已啟用狀態變更為停用）：
 *      {"Device": "B8D61A68DAFC", "Card": "1122334455", "Result": "Card Invalided"}
 *      說明：該卡片已成功停用，將無法再開啟門鎖
 * 
 *   3. 卡片不存在（資料庫中無此卡片記錄）：
 *      {"Device": "B8D61A68DAFC", "Card": "1122334455", "Result": "Card Not Existed"}
 *      說明：該卡片尚未在雲端資料庫中註冊，無法停用
 * 

 * 
 * 【SendtoClouding() 函式處理流程】
 *   此函式定義於 clouding.h 中，會執行以下操作：
 *   1. 組合 HTTP GET 請求字串（包含 MAC 位址與 RFID 卡號）
 *   2. 將請求傳送至 disablerfid.php API
 *   3. 接收伺服器回傳的 JSON 資料
 *   4. 解析 JSON 資料，提取驗證結果（Device、Card、Result）
 *   5. 根據回傳結果輸出對應訊息至序列監控視窗：
 *      - "Device Not Existed"：設備未註冊
 *      - "Card Invalided"：卡片已成功停用
 *      - "Card Not Existed"：卡片不存在於資料庫
 * =============================================================================
 */

// 門禁裝置讀取卡號傳送雲端停用程式 invalidRFID_inrfidlist_BMduino.ino

// ========================================
// 全域變數定義
// ========================================
String SSIDData ;           // 儲存所連接的 Wi-Fi AP（無線基地台）的 SSID 名稱
String IPData ;             // 儲存開發板取得的 IP 位址字串（例如 "192.168.1.100"）
String MacData ;            // 儲存開發板的 MAC 位址字串（例如 "AA:BB:CC:DD:EE:FF"）
//String uidStr ;           // 儲存讀取到的 RFID 卡片 UID 字串（已移至 clouding.h 定義）

// ========================================
// 外部函式庫引入
// ========================================
#include <String.h>         // 引入字串處理函式庫，用於字串的儲存與操作
#include "TCP.h"            // 引入 TCP 通訊函式庫，包含網路連線相關的函式
#include "RFIDLib.h"        // 引入 RFID 讀卡模組函式庫，用於 BMC11T001 RFID 讀寫模組的通訊控制
#include "clouding.h"       // 引入雲端通訊函式庫，提供 HTTP GET 方式將資料傳送至雲端伺服器的功能

// ========================================
// 函式前置宣告
// ========================================
void initSensor();          // 初始化所有感測模組（RFID、OLED、繼電器、Wi-Fi）
void initAll();             // 初始化整體系統（序列埠、感測器、LED 狀態等）
void GetWiFiInformation();  // 取得 WiFi 資訊
void INITWIFI();            // 初始化 Wi-Fi 網路連線
void ShowWiFiInformation(); // 顯示 Wi-Fi 連線資訊（MAC、SSID、IP）

// ========================================
// setup() 函式：Arduino 啟動時只執行一次
// ========================================
void setup() 
{
  // 步驟 1：初始化整體系統
  // 包含序列埠通訊、感測器模組（Wi-Fi、RFID）等初始化工作
  initAll();          // 初始化整體系統（序列埠、繼電器、OLED、Wi-Fi、RFID 等模組）
  Serial.println("Enter Loop() Area and Start to Work"); // 輸出開始工作的提示訊息
}

// ========================================
// loop() 函式：Arduino 啟動後會重複執行此區塊的程式碼
// ========================================
void loop()
{
  // 步驟 1：檢查是否有 RFID 卡片靠近並成功讀取
  // checkReadRFIDSuccess() 回傳 true 表示偵測到卡片且讀取成功
  // 此函式會自動處理 I2C 通訊與卡片偵測邏輯
  if (checkReadRFIDSuccess())
  {
    // 步驟 2：讀取 RFID 卡片的唯一識別碼（UID）
    // readRFIDUIDString() 會從 RFID 模組讀取卡片 UID 並轉換為字串格式
    // 例如：讀取到的 UID 可能為 "0079262864" 或 "1122334455"
    uidStr = readRFIDUIDString();     // 讀取 RFID 卡片的 UID 並轉換為字串格式

    // 步驟 3：將讀取到的卡號輸出至序列監控視窗
    // 此為除錯用途，可確認 RFID 讀卡模組是否正常運作
    Serial.print("read RFID Card Number is :(");
    Serial.print(uidStr);
    Serial.print(")\n");

    // Wifi.getStatus() 回傳 true 表示 Wi-Fi 已連線且運作正常
    // 若 Wi-Fi 未連線，則無法將資料傳送至雲端伺服器
    if (Wifi.getStatus())
    {
      // 步驟 4a：Wi-Fi 連線正常，輸出確認訊息
      Serial.println("WIFI OK");      // 輸出 Wi-Fi 連線正常訊息至序列監控視窗
      
      // 步驟 4b：呼叫 SendtoClouding() 函式傳送資料至雲端
      // 此函式定義於 clouding.h 中，會執行以下操作：
      //   - 組合 HTTP GET 請求（包含 MAC 位址與 RFID 卡號）
      //   - 將請求傳送至雲端伺服器（disablerfid.php）
      //   - 接收並解析伺服器回傳的 JSON 資料
      //   - 根據回傳結果判斷卡片停用狀態
      SendtoClouding();               // 呼叫 SendtoClouding() 函式，將感測資料（卡號）透過 HTTP 傳送至雲端伺服器
    }
    else
    {
      // 步驟 4c：Wi-Fi 未連線，嘗試重新連線
      // INITWIFI() 會執行以下操作：
      //   - 呼叫 GetWiFiInformation() 取得目前 WiFi 資訊
      //   - 呼叫 initWiFi() 連線至預先設定的無線網路
      //   - 呼叫 ShowWiFiInformation() 顯示連線資訊（MAC、SSID、IP）
      INITWIFI();          // 初始化 WiFi 網路
      
      // 重新檢查 Wi-Fi 連線狀態
      if (Wifi.getStatus())
      {
        // 重新連線成功，輸出確認訊息
        Serial.println("WIFI OK");      // 輸出 Wi-Fi 連線正常訊息至序列監控視窗
        
        // 呼叫 SendtoClouding() 函式傳送資料至雲端
        SendtoClouding();               // 呼叫 SendtoClouding() 函式，將感測資料（卡號）透過 HTTP 傳送至雲端伺服器
      }
      // 若重新連線後仍失敗，則放棄本次傳送，等待下次卡片感應
    }
  }

  // 步驟 5：延遲 2 秒鐘（2000 毫秒）再進行下一次的讀卡動作
  // 延遲的目的：
  //   1. 避免因程式循環過快導致重複讀取同一張卡片
  //   2. 避免短時間內發送過多 HTTP 請求造成雲端伺服器過度負載
  //   3. 給予使用者足夠的時間移開卡片，防止重複觸發
  // 可根據實際需求調整延遲時間，例如 1000 毫秒（1 秒）或 3000 毫秒（3 秒）
  delay(2000);
}

// ========================================
// initSensor() 函式：初始化所有感測模組
// ========================================
void initSensor()
{
  // 步驟 1：初始化 Wi-Fi 網路
  // INITWIFI() 會執行以下操作：
  //   - 呼叫 GetWiFiInformation() 取得目前 WiFi 資訊
  //   - 呼叫 initWiFi() 連線至預先設定的無線網路
  //   - 呼叫 ShowWiFiInformation() 顯示連線資訊（MAC、SSID、IP）
  INITWIFI();          // 初始化 WiFi 網路
                       // 此函式應包含 SSID 與密碼的設定，以及連線至無線網路
  
  // 步驟 2：初始化 RFID 讀卡模組
  // initRFID() 會設定 I2C 通訊介面，準備讀取 RFID 卡片 UID
  // 若 RFID 模組初始化失敗，可能需要在序列監控視窗輸出錯誤訊息
  initRFID();          // 初始化 RFID 讀卡模組（BMC11T001）
                       // 設定 I2C 通訊，準備讀取 RFID 卡片 UID
}

// ========================================
// initAll() 函式：初始化整體系統
// ========================================
void initAll()
{
  // 步驟 1：啟動序列埠通訊
  // 設定傳輸速率為 9600 bps，此速率需與 Arduino IDE 序列監控視窗設定的鮑率一致
  // 若鮑率不匹配，序列監控視窗將顯示亂碼或無法顯示任何訊息
  Serial.begin(9600);               // 啟動序列埠通訊，設定傳輸速率為 9600 bps
                                    // 此速率需與 Arduino IDE 序列監控視窗設定的鮑率一致
  
  // 步驟 2：設定內建 LED 狀態
  // 將內建 LED 腳位設定為 LOW 電位（熄滅）
  // 此處可根據需求修改 LED 的初始狀態：
  //   - 可設定為 HIGH 表示開機指示燈亮起
  //   - 可加入閃爍效果表示系統啟動中
  digitalWrite(LED, LOW);           // 將內建 LED 腳位設定為 LOW 電位（熄滅）
                                    // 作為開機狀態指示，可根據實際需求修改為閃爍或其他模式
  
  // 步驟 3：初始化所有感測器模組
  // 包含 Wi-Fi 連線與 RFID 讀卡模組的初始化
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