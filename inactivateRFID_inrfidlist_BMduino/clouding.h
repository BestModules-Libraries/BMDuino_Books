/*
 * =============================================================================
 * 檔案名稱: clouding.h
 * 程式語言: Arduino C/C++
 * 檔案類型: 標頭檔（Header File）
 * 功能說明: 門禁卡號傳送雲端停用卡片程式（停用卡片版本），負責 HTTP 請求與 JSON 資料解析
 * 門禁卡號傳送雲端停用卡片程式
 * =============================================================================
 * 程式運作流程完整解說
 * =============================================================================
 * 
 * 【檔案概述】
 *   本檔案為雲端門禁系統的雲端通訊模組（停用卡片版本），負責將 RFID 讀取到的卡號
 *   透過 HTTP GET 請求傳送至遠端雲端伺服器進行停用作業，並解析伺服器回傳的
 *   JSON 資料，根據停用結果輸出對應訊息。
 * 
 * 【主要功能】
 *   1. 組成 HTTP GET 請求字串，包含設備 MAC 位址與 RFID 卡號
 *   2. 透過 Wi-Fi 模組將請求傳送至雲端伺服器的 disablerfid.php API
 *   3. 接收伺服器回傳的 JSON 格式回應
 *   4. 解析 JSON 資料，提取驗證結果（Device、Card、Result）
 *   5. 根據停用結果輸出對應訊息至序列監控視窗
 *   6. 可擴充 OLED 顯示與門鎖控制等功能
 * 
 * 【與啟用版本 clouding.h 的差異說明】
 *   本檔案與啟用卡片版本的 clouding.h 主要差異如下：
 *   - API 路徑：使用 disablerfid.php（停用）而非 enablerfid.php（啟用）
 *   - 回傳結果：處理 "Card Inactivated"（卡片已停用）而非 "Card Activated"
 *   - 功能目的：撤銷卡片權限而非授予權限
 * 
 * 【雲端伺服器規格】
 *   - 伺服器網址：http://iot.arduino.org.tw
 *   - 通訊埠：8888
 *   - API 路徑：/bmduino/rfid/disablerfid.php
 *   - 請求參數：
 *        MAC：設備的 MAC 位址（唯一識別碼）
 *        KEY：讀取到的 RFID 卡號
 *   - 請求範例：
 *        http://iot.arduino.org.tw:8888/bmduino/rfid/disablerfid.php?MAC=483FDACA32A3&KEY=0079262864
 * 
 * 【JSON 回應格式說明】
 *   伺服器回傳的 JSON 資料格式有以下幾種情況：
 * 
 *   1. 裝置不存在：
 *      {
 *          "Device": "B8D61A68DAFC",
 *          "Card": "1122334455",
 *          "Result": "Device Not Existed"
 *      }
 *      說明：此設備尚未在雲端系統中註冊或已被停用
 * 
 *   2. 卡片不存在：
 *      {
 *          "Device": "B8D61A68DAFC",
 *          "Card": "1122334455",
 *          "Result": "Card Not Existed"
 *      }
 *      說明：該卡片尚未在雲端資料庫中註冊，無法停用
 * 
 *   3. 卡片已停用：
 *      {
 *          "Device": "B8D61A68DAFC",
 *          "Card": "1122334455",
 *          "Result": "Card Inactivated"
 *      }
 *      說明：該卡片已成功停用，將無法再開啟門鎖
 * 
 * 【整體運作流程圖】
 * 
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                    SendtoClouding() 函式開始                 │
 *   └─────────────────────────────────────────────────────────────┘
 *                                │
 *                                ▼
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │ 步驟 1：組成 HTTP GET 請求字串                               │
 *   │   sprintf(dbagentstr, dbagent, MacData, uidStr)            │
 *   │   組合結果範例：                                             │
 *   │   "/bmduino/rfid/disablerfid.php?MAC=483FDACA32A3&KEY=...   │
 *   └─────────────────────────────────────────────────────────────┘
 *                                │
 *                                ▼
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │ 步驟 2：檢查 Wi-Fi 連線狀態                                  │
 *   │   if (Wifi.getStatus())                                    │
 *   └─────────────────────────────────────────────────────────────┘
 *                     │                          │
 *                     │ 是                       │ 否
 *                     ▼                          ▼
 *   ┌────────────────────────────┐   ┌────────────────────────────┐
 *   │ 步驟 3：執行 HTTP GET 請求  │   │ 跳過通訊，不執行任何操作    │
 *   │   Wifi.http_begin()        │   │ （可加入錯誤處理）          │
 *   │   Wifi.http_get()          │   └────────────────────────────┘
 *   │   delay(500)               │              │
 *   │   webresponse = http_getString()           │
 *   └────────────────────────────┘              │
 *                     │                          │
 *                     ▼                          │
 *   ┌─────────────────────────────────────────────┐
 *   │ 步驟 4：處理 JSON 回應資料                   │
 *   │   webresponse = getjson(webresponse)       │
 *   │   Wifi.http_end()                          │
 *   └─────────────────────────────────────────────┘
 *                     │
 *                     ▼
 *   ┌─────────────────────────────────────────────┐
 *   │ 步驟 5：解析 JSON 資料                       │
 *   │   deserializeJson(doc, webresponse)        │
 *   └─────────────────────────────────────────────┘
 *                     │
 *                     ▼
 *   ┌─────────────────────────────────────────────┐
 *   │ 步驟 6：提取 JSON 欄位值                     │
 *   │   device = doc["Device"]                    │
 *   │   Card = doc["Card"]                        │
 *   │   result = doc["Result"]                    │
 *   └─────────────────────────────────────────────┘
 *                     │
 *                     ▼
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │ 步驟 7：根據停用結果進行對應處理                             │
 *   │                                                             │
 *   │   if (Result == "Device Not Existed")                       │
 *   │       輸出：設備未註冊                                       │
 *   │                                                             │
 *   │   if (Result == "Card Inactivated")                         │
 *   │       輸出：卡片已停用                                       │
 *   │                                                             │
 *   │   if (Result == "Card Not Existed")                         │
 *   │       輸出：卡片不存在                                       │
 *   └─────────────────────────────────────────────────────────────┘
 *                     │
 *                     ▼
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                    SendtoClouding() 函式結束                 │
 *   └─────────────────────────────────────────────────────────────┘
 * 
 * 【關鍵函式說明】
 *   - SendtoClouding()：核心函式，負責 HTTP 通訊與 JSON 解析
 *   - getjson()：過濾或提取純 JSON 內容（定義於 JSONLib.h）
 *   - deserializeJson()：ArduinoJson 函式庫的 JSON 解析函式
 * 
 * 【通訊協定說明】
 *   使用 HTTP GET 方法進行通訊，將參數直接附加於 URL 之後，
 *   伺服器端以 PHP 程式接收並處理請求，回傳 JSON 格式的停用結果。
 * 
 * 【錯誤處理機制】
 *   - 若 Wi-Fi 未連線，則不執行任何通訊操作
 *   - 若 JSON 解析失敗，則輸出錯誤訊息並中斷後續處理
 *   - 可依需求加入重試機制或錯誤提示
 * 
 * 【外部函式庫說明】
 * 
 *   【JSONLib.h】
 *   提供 JSON 字串的解析與處理功能，可能封裝了 ArduinoJson 函式庫，
 *   其中 getjson() 函式用於過濾或提取純 JSON 內容。
 * 
 *   【Wi-Fi 通訊模組】
 *   透過 Wifi 物件進行網路連線與 HTTP 通訊，提供以下方法：
 *     - getStatus()：檢查 Wi-Fi 連線狀態
 *     - http_begin()：初始化 HTTP 連線
 *     - http_get()：執行 HTTP GET 請求
 *     - http_getString()：取得 HTTP 回應內容
 *     - http_end()：結束 HTTP 連線
 * 
 * 【全域變數說明】
 *   - ServerURL：雲端伺服器的基礎網址
 *   - ServerPort：雲端伺服器的通訊埠號（8888）
 *   - dbagent：API 路徑與參數的格式字串（注意：此處使用 enablerfid.php 需修正）
 *   - dbagentstr：sprintf() 組合字串時的暫存區
 *   - connectstr：動態組成的完整 RESTful 請求參數
 *   - uidStr：儲存讀取到的 RFID 卡號字串
 *   - webresponse：儲存伺服器回傳的原始回應字串
 *   - jsonresult：儲存解析後的 Result 欄位值
 * 
 * 【使用注意事項】
 *   1. 使用前請確認 TCP.h 中已正確設定 Wi-Fi SSID 與密碼
 *   2. 確認雲端伺服器位址與埠號設定正確（預設連接埠 8888）
 *   3. 本檔案依賴 JSONLib.h 與 ArduinoJson 函式庫
 *   4. MacData（設備 MAC 位址）需由主程式定義並賦值
 *   5. doc（JSON 文件物件）需在其他地方宣告（如 StaticJsonDocument）
 *   6. ⚠️ 重要：本檔案中的 dbagent 仍使用 enablerfid.php，若要用於停用卡片，
 *      請將 "#define dbagent ... enablerfid.php ..." 修改為：
 *      "#define dbagent "/bmduino/rfid/disablerfid.php?MAC=%s&KEY=%s""
 * 
 * 版本: 1.0
 * 建立日期: 2026-03-30
 * =============================================================================
 */

// 原始雲端 API 路徑範例（僅供參考，實際使用下方定義的常數）
// http://iot.arduino.org.tw:8888/bmduino/rfid/disablerfid.php?MAC=483FDACA32A3&KEY=0079262864

// ========================================
// 外部函式庫引入
// ========================================
// #include "commlib.h"  // common lib 元件（註解狀態，未使用）
#include "JSONLib.h"          // 引入 JSON 解析函式庫，用於處理伺服器回傳的 JSON 資料
                              // 此函式庫可能封裝了 ArduinoJson 的相關功能

// ========================================
// 常數與巨集定義
// ========================================
#define  HTTPGET_PORT_HTTP 80      // HTTP 通訊協定的預設埠號（80）
#define  HTTPGET_PORT_HTTPS 443    // HTTPS 通訊協定的預設埠號（443）

#define ServerPort 8888            // 雲端伺服器的通訊埠號
String ServerURL = "http://iot.arduino.org.tw";  // 雲端伺服器的基礎網址

// dbagent：API 路徑與參數的格式字串
// %s 為格式化的佔位符，會依序被 MacData 與 uidStr 取代
// 注意：此處使用 enablerfid.php（啟用 API），若需停用卡片請改為 disablerfid.php
#define dbagent "/bmduino/rfid/disablerfid.php?MAC=%s&KEY=%s"

// ========================================
// 全域變數定義
// ========================================
char dbagentstr[300];              // sprintf() 組合字串時的暫存區，儲存完整的 API 路徑
String connectstr;                 // 一個空的字串變數，後續用來動態組成完整的 RESTful 請求參數
String uidStr = "";                // 儲存讀取到的 RFID 卡號字串（例如 "0079262864"）
String webresponse;                // 儲存 HTTP GET 請求後，伺服器回傳的原始回應字串
String jsonresult;                 // 儲存解析後的 JSON 結果（例如 "Find" 或 "notFind"）

/*
 * HTTP 請求結構說明：
 * 
 * 完整的 HTTP 請求範例，分解如下：
 *   主機位址：iot.arduino.org.tw:8888（域名與通訊埠）
 *   應用程式路徑：/bmduino/rfid/disablerfid.php（伺服器上的 PHP 程式，用來接收並處理資料）
 *   參數：?MAC=B8D61A68DAFC&KEY=483FDACA32A3，表示傳送的資料：
 *        MAC：設備的唯一識別碼（MAC 位址）
 *        KEY：RFID 卡號
 * 
 * 完整 URL 範例：
 *   http://iot.arduino.org.tw:8888/bmduino/rfid/disablerfid.php?MAC=483FDACA32A3&KEY=0079262864
 */

// ========================================
// 函式前置宣告
// ========================================
void SendtoClouding();             // 傳送感測資料（RFID 卡號）到雲端伺服器
void PrintCardonOLED(String ss);   // 顯示卡號在 OLED 螢幕上（此函式應由主程式實作）
void PrintmsgonOLED(String ss);    // 顯示結果資訊在 OLED 螢幕上（此函式應由主程式實作）

// ========================================
// SendtoClouding() 函式：將 RFID 卡號傳送至雲端伺服器進行停用驗證
// 此函式為本檔案的核心功能，負責 HTTP 通訊與 JSON 解析
// ========================================
void SendtoClouding()     
{
  // 函式說明：
  //   此函式將讀取到的 RFID 卡號（uidStr）與設備 MAC 位址（MacData）
  //   透過 HTTP GET 請求傳送到雲端伺服器，並根據回傳結果管理系統

  // ========================================
  // 步驟 1：組成 HTTP GET 請求的參數字串
  // ========================================
  // 使用 sprintf() 將 MacData 與 uidStr 填入 dbagent 的格式字串中
  // dbagentstr 會儲存完整路徑，例如：
  //   "/bmduino/rfid/disablerfid.php?MAC=483FDACA32A3&KEY=0079262864"
  sprintf(dbagentstr, dbagent, MacData.c_str(), uidStr.c_str());
  
  // 將組合好的路徑字串轉換為 String 型態，儲存於 connectstr
  connectstr = String(dbagentstr);
  
  /*
   * 組成 GET 格式的 RESTful 請求參數字串
   * connectstr：動態組成 RESTful 請求的參數部分
   * MacData：設備的 MAC 位址
   * uidStr：讀取到的 RFID 卡號
   */

  // 將組合好的參數字串輸出至序列監控視窗，用於除錯
  Serial.println(connectstr);

  // ========================================
  // 步驟 2：檢查 Wi-Fi 連線狀態
  // ========================================
  if (Wifi.getStatus())  // 若 Wi-Fi 連線正常，回傳 true
  {
    // ========================================
    // 步驟 3：執行 HTTP GET 請求
    // ========================================
    // 開始 HTTP 連線，參數說明：
    //   ServerURL：伺服器基礎網址（http://iot.arduino.org.tw）
    //   ServerPort：伺服器埠號（8888）
    //   connectstr：完整的 API 路徑與參數
    Wifi.http_begin(ServerURL, ServerPort, connectstr);

    // 執行 HTTP GET 操作
    Wifi.http_get();
    
    // 延遲 500 毫秒，等待伺服器回應
    // 此延遲確保伺服器有足夠時間處理請求並回傳資料
    delay(1000);
    
    // 取得 HTTP 回應內容（伺服器回傳的 JSON 字串）
    webresponse = Wifi.http_getString();
    
    // 輸出伺服器回應內容至序列監控視窗（除錯用）
    Serial.print("web response:[");
    Serial.print(webresponse);
    Serial.print("]\n");
    
    // 輸出回應字串的長度（除錯用）
    Serial.print("webresponse len is :(");
    Serial.print(webresponse.length());
    Serial.print(")\n");
    
    // ========================================
    // 步驟 4：處理 JSON 回應資料
    // ========================================
    // 呼叫 getjson() 函式過濾或提取 JSON 內容
    // 註：getjson() 可能定義於 JSONLib.h 中，用於純化 JSON 字串
    // 此步驟可去除可能存在的 HTML 標籤或空白字元
    webresponse = getjson(webresponse);
    
    // 輸出處理後的 JSON 字串（除錯用）
    Serial.print("New webresponse len is :(");
    Serial.print(webresponse);
    Serial.print(")\n");
    
    // 結束 HTTP 連線，釋放資源
    Wifi.http_end();

    // ========================================
    // 步驟 5：解析 JSON 資料
    // ========================================
    // 使用 ArduinoJson 函式庫解析 JSON 字串
    // doc 物件應已在其他地方宣告（例如 StaticJsonDocument 或 DynamicJsonDocument）
    DeserializationError error = deserializeJson(doc, webresponse);

    // 檢查 JSON 解析是否成功
    if (error) {
      // 解析失敗，輸出錯誤訊息
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;  // 結束函式，不繼續執行後續處理
    }

    // ========================================
    // 步驟 6：提取 JSON 欄位值
    // ========================================
    // 從解析後的 JSON 文件中提取各欄位的值
    const char* device = doc["Device"];   // 提取 "Device" 欄位（應為 MAC 位址）
    const char* Card = doc["Card"];       // 提取 "Card" 欄位（應為 RFID 卡號）
    const char* result = doc["Result"];   // 提取 "Result" 欄位（停用結果狀態）
    
    // 將 Result 欄位轉換為 String 型態，儲存於全域變數
    jsonresult = String(result);

    // ========================================
    // 步驟 7：根據停用結果進行對應處理
    // ========================================
    // 以下判斷式根據雲端伺服器回傳的結果進行對應的處理
    // 目前僅輸出對應訊息至序列監控視窗，可依需求擴充門鎖控制等功能

    // 情況一：設備未註冊
    // 當回傳的 Device 與本機 MAC 相符，且 Result 為 "Device Not Existed" 時
    // 表示此設備尚未在雲端系統中註冊，無法進行門禁驗證
    if (String(device) == MacData && String(result) == "Device Not Existed") 
    {
      // 輸出設備未註冊的錯誤訊息
      Serial.print("Security DEVICE:(");
      Serial.print(MacData);
      Serial.print(") is not Registered and Fail\n");
    }
    
    // 情況二：卡片已停用
    // 當回傳的 Device 與本機 MAC 相符，且 Result 為 "Card Inactivated" 時
    // 表示此 RFID 卡片的權限已在雲端系統中成功停用
    if (String(device) == MacData && String(result) == "Card Inactivated") 
    {
      // 輸出卡片已成功停用的訊息
      Serial.print("RFID Card (");
      Serial.print(uidStr);
      Serial.print(") is Inactivated \n");
    }
    
    // 情況三：卡片不存在於資料庫
    // 當回傳的 Device 與本機 MAC 相符，且 Result 為 "Card Not Existed" 時
    // 表示此 RFID 卡片尚未在雲端資料庫中註冊，無法停用
    if (String(device) == MacData && String(result) == "Card Not Existed") 
    {
      // 輸出卡片不存在的錯誤訊息
      Serial.print("RFID Card (");
      Serial.print(uidStr);
      Serial.print(") is not existed in database \n");
    }

  }
  // 若 Wi-Fi 未連線，則不執行任何操作（可依需求加入錯誤處理）
  // 此處可考慮加入錯誤提示訊息或重試機制
}