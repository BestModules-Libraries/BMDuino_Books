/*
 * 檔案名稱：clouding.h
 * 程式語言：Arduino C/C++
 * 檔案類型：標頭檔（Header File）
 * 
 * ========================================
 * 程式功能完整解說
 * ========================================
 * 
 * 【檔案概述】
 *   本檔案為雲端門禁系統的雲端通訊模組，負責將 RFID 讀取到的卡號
 *   透過 HTTP GET 請求傳送至遠端雲端伺服器，並解析伺服器回傳的
 *   JSON 資料，根據驗證結果決定是否開啟門鎖。
 * 
 * 【主要功能】
 *   1. 組成 HTTP GET 請求字串，包含設備 MAC 位址與 RFID 卡號
 *   2. 透過 Wi-Fi 模組將請求傳送至雲端伺服器
 *   3. 接收伺服器回傳的 JSON 格式回應
 *   4. 解析 JSON 資料，提取驗證結果（Device、Card、Result）
 *   5. 根據驗證結果控制門鎖動作（開啟/關閉）
 *   6. 將結果顯示於 OLED 螢幕及序列監控視窗
 * 
 * 【雲端伺服器規格】
 *   - 伺服器網址：http://iot.arduino.org.tw
 *   - 通訊埠：8888
 *   - API 路徑：/bmduino/rfid/requestpass.php
 *   - 請求參數：
 *        MAC：設備的 MAC 位址（唯一識別碼）
 *        KEY：讀取到的 RFID 卡號
 *   - 請求範例：
 *        http://iot.arduino.org.tw:8888/bmduino/rfid/requestpass.php?MAC=483FDACA32A3&KEY=0230205122
 * 
 * 【JSON 回應格式】
 *   伺服器回傳的 JSON 資料格式範例：
 *   {
 *     "Device": "112233445566",
 *     "Card": "0079262864",
 *     "Result": "Find"        // "Find" 表示卡號已註冊，"notFind" 表示未註冊
 *   }
 * 
 * 【運作流程】
 *   1. 外部程式（主程式）呼叫 SendtoClouding() 函式
 *   2. 使用 sprintf() 將 MAC 位址與卡號組合成完整請求路徑
 *   3. 檢查 Wi-Fi 連線狀態（Wifi.getStatus()）
 *   4. 若 Wi-Fi 已連線，透過 HTTP GET 方式傳送請求
 *   5. 接收伺服器回應，並儲存於 webresponse 字串
 *   6. 呼叫 getjson() 函式解析 JSON 資料（或直接使用 ArduinoJson 解析）
 *   7. 使用 deserializeJson() 解析 JSON 字串
 *   8. 提取 Device、Card、Result 三個欄位的值
 *   9. 比對 Device 是否與本機 MAC 位址相符
 *   10. 根據 Result 值決定門禁權限：
 *        - 若 Result 為 "Find"：卡號已註冊，開啟門鎖 2 秒後關閉
 *        - 若 Result 為 "notFind"：卡號未註冊，不開啟門鎖
 *   11. 將驗證結果顯示於 OLED 螢幕
 * 
 * 【相依函式庫】
 *   - JSONLib.h：JSON 解析函式庫（可能為 ArduinoJson 的封裝）
 *   - Wifi 物件：來自 RelayLib 或 TCP.h，提供網路連線功能
 * 
 * 【全域變數說明】
 *   - MacData：儲存設備的 MAC 位址（由主程式傳入或取得）
 *   - uidStr：儲存讀取到的 RFID 卡號字串
 *   - dbagentstr：sprintf 組合字串時的暫存區
 *   - connectstr：組合完成的完整 HTTP 請求路徑
 *   - webresponse：儲存伺服器回傳的原始回應字串
 *   - jsonresult：儲存解析後的 JSON 結果（Result 欄位值）
 * 
 * 【注意事項】
 *   - 使用前需確保 MacData 與 uidStr 已正確賦值
 *   - Wi-Fi 模組需已完成初始化並成功連線
 *   - 伺服器端需正確佈署 checkpass.php 程式
 *   - JSON 解析失敗時會輸出錯誤訊息至序列監控視窗
 *   - 開啟門鎖的繼電器控制函式（openDoor()、closeDoor()）需事先定義
 * 
 * 【版本資訊】
 *   最後修改日期：2026年3月26日
 *   適用專案：註冊RFID裝置之雲端門禁裝置
 */

// http://iot.arduino.org.tw:8888/bmduino/rfid/requestpass.php?MAC=483FDACA32A3&KEY=0230205122
// 以上為原始的雲端 API 路徑範例（註解）

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
#define dbagent "/bmduino/rfid/requestpass.php?MAC=%s&KEY=%s"

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
 *   應用程式路徑：/bmduino/rfid/checkpass.php（伺服器上的 PHP 程式，用來接收並處理資料）
 *   參數：?MAC=B8D61A68DAFC&KEY=0079262864，表示傳送的資料：
 *        MAC：設備的唯一識別碼（MAC 位址）
 *        KEY：RFID 卡號
 * 
 * 完整 URL 範例：
 *   http://iot.arduino.org.tw:8888/bmduino/rfid/checkpass.php?MAC=112233445566&KEY=0079262864
 */

// ========================================
// 函式前置宣告
// ========================================
void SendtoClouding();             // 傳送感測資料（RFID 卡號）到雲端伺服器
void PrintCardonOLED(String ss);   // 顯示卡號在 OLED 螢幕上（此函式應由主程式實作）
void PrintmsgonOLED(String ss);    // 顯示結果資訊在 OLED 螢幕上（此函式應由主程式實作）

// ========================================
// SendtoClouding() 函式：將 RFID 卡號傳送至雲端伺服器進行驗證
// 此函式為本檔案的核心功能，負責 HTTP 通訊與 JSON 解析
// ========================================
void SendtoClouding()     
{
  // 函式說明：
  //   此函式將讀取到的 RFID 卡號（uidStr）與設備 MAC 位址（MacData）
  //   透過 HTTP GET 請求傳送到雲端伺服器，並根據回傳結果控制門鎖動作。

  // ========================================
  // 步驟 1：組成 HTTP GET 請求的參數字串
  // ========================================
  // 使用 sprintf() 將 MacData 與 uidStr 填入 dbagent 的格式字串中
  // dbagentstr 會儲存完整路徑，例如：
  //   "/bmduino/rfid/checkpass.php?MAC=112233445566&KEY=0079262864"
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
    delay(500);
    
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
    webresponse = getjson(webresponse);
    
    // 輸出處理後的 JSON 字串長度（除錯用）
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
      return;  // 結束函式，不繼續執行後續門鎖控制
    }

    // ========================================
    // 步驟 6：提取 JSON 欄位值
    // ========================================
    // 從解析後的 JSON 文件中提取各欄位的值
    const char* device = doc["Device"];   // 提取 "Device" 欄位（應為 MAC 位址）
    const char* Card = doc["Card"];       // 提取 "Card" 欄位（應為 RFID 卡號）
    const char* result = doc["Result"];   // 提取 "Result" 欄位（"Find" 或 "notFind"）
    
    // 將 Result 欄位轉換為 String 型態，儲存於全域變數
    jsonresult = String(result);

    // ========================================
    // 步驟 7：根據驗證結果控制門鎖
    // ========================================
    // 判斷條件：Device 欄位與本機 MAC 位址相符，且 Result 為 "Device Not Existed"
    if (String(device) == MacData && String(result) == "Device Not Existed") 
    {
      // 卡號已註冊，允許通行
      Serial.print("RFID LOCK DEVICE:()");
      Serial.print(MacData);
      Serial.print(") is not Registered and Fail\n");
      
      // 在 OLED 螢幕上顯示驗證結果 "Find"
      PrintmsgonOLED("noFind");
      
      // 開啟門鎖（控制繼電器）
      closeDoor();     // 控制門鎖繼電器，設定為關閉狀態
    }
        
    // 判斷條件：Device 欄位與本機 MAC 位址相符，且 Result 為 "Find"
    if (String(device) == MacData && String(result) == "Find") 
    {
      // 卡號已註冊，允許通行
      Serial.print("RFID LOCK DEVICE:()");
      Serial.print(MacData);
      Serial.print(") Registered and PASS\n");
      
      // 在 OLED 螢幕上顯示驗證結果 "Find"
      PrintmsgonOLED("Find");
      
      // 開啟門鎖（控制繼電器）
      openDoor();      // 控制門鎖繼電器，設定為開啟狀態
      
      // 延遲 2 秒鐘，讓門鎖保持開啟狀態足夠時間讓人員通過
      delay(2000);
      
      // 關閉門鎖
      closeDoor();     // 控制門鎖繼電器，設定為關閉狀態
    }
    
    // 判斷條件：Device 欄位與本機 MAC 位址相符，但 Result 為 "notFind"
    if (String(device) == MacData && String(result) == "notFind") 
    {
      // 卡號未註冊，拒絕通行
      Serial.print("RFID LOCK DEVICE:()");
      Serial.print(MacData);
      Serial.print(") is not Registered and NO PASS \n");
      
      // 在 OLED 螢幕上顯示驗證結果 "notFind"
      PrintmsgonOLED("notFind");
      // 注意：此處未呼叫 openDoor()，因此門鎖保持關閉狀態
    }
  }
  // 若 Wi-Fi 未連線，則不執行任何操作（可依需求加入錯誤處理）
}