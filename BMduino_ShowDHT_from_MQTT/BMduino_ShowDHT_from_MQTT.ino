// ================================================================
// =============== 全域變數宣告區 (Global Variables) ===============
// ================================================================

// 宣告三個全域變數，用於儲存 WiFi 模組相關資訊
String MacData;   // 儲存目前裝置的 WiFi MAC Address（網路卡的唯一識別碼）
String SSIDData;  // 儲存目前連線的 WiFi 熱點名稱 (SSID)
String IPData;    // 儲存 WiFi 連線後由路由器分配的 IP 位址

// ------- 感測模組函式與外部函式引用宣告區 -----------
#include <String.h>    // Arduino 內建字串處理函式庫
#include "commlib.h"   // 通訊相關的共用函式庫（可能包含封包處理、緩衝區管理等）
#include "OledLib.h"   // 自訂 OLED 顯示模組函式庫（提供 OLED 初始化、文字繪製、清屏等功能）
#include "TCP.h"       // TCP 通訊函式庫（包含 WiFi 初始化、MAC 取得等函式）
#include "MQTTLib.h"   // MQTT 函式庫，提供 MQTT 初始化、連線與發佈/訂閱訊息功能

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統
void INITWIFI();                   // 初始化 WiFi 網路連線
void showTemperatureonOled(float ss); // 列印溫度於 OLED 上
void showHumidityonOled(float ss);  // 列印濕度於 OLED 上

// ---------- 系統初始化函式 (Arduino 啟動時執行一次) ----------
void setup()
{
  initAll();       // 初始化整體系統（啟動序列埠、初始化 OLED、Relay）
  delay(200);      // 延遲 200ms，確保硬體模組穩定
  
  INITWIFI();      // 初始化 WiFi 網路，並取得 SSID、IP 與 MAC 資料

  // 檢查 WiFi 連線狀態
  if (Wifi.getStatus())   // 確認 WiFi 是否連線成功
  {
    initMQTT();           // 初始化 MQTT 伺服器連線
    Serial.println("WIFI OK"); // 在序列埠顯示連線成功訊息
    // SendtoClouding();  // （選用功能）傳送感測資料到雲端（目前註解）
  }

  //---------------------------------
  clearScreen();  // 清除 OLED 螢幕
  // 顯示 BEST MODULES 的 LOGO 圖片
  drawPicture(0, 0, BestModule_LOGO, 128, 64); // 在 OLED 左上角繪製 LOGO
  delay(3000);    // LOGO 顯示 3 秒
  clearScreen();  // 再次清除螢幕
  
  // 顯示系統資訊於 OLED
  showTitleonOled(MacData, 0);  // 在 OLED 第一行顯示 MAC 位址
  //----------------------------
  Serial.println("Enter Loop()"); // 提示已經進入主迴圈 loop()
}

// ------------------ 主迴圈 (Arduino 持續重複執行) ------------------
void loop() 
{              
  // WiFi 斷線檢查與重連邏輯（目前註解掉，可依需求開啟）
  // 當 WiFi 斷線時可重新執行 INITWIFI()

  // 從 MQTT Broker 接收資料
  // 讀取 Broker 下發的資料，包含訊息內容、長度與主題
  Wifi.readIotData(&ReciveBuff, &ReciveBufflen, &topic);
  //  {"Device":"083A8DF11676","Temperature":26.8,"Humidity":78.9}
  // for publish topic:/arduino/dht/083A8DF11676 Testing
  // 檢查是否有接收到資料（ReciveBufflen > 0 表示有資料）
  if (ReciveBufflen)   
  {
    // 在序列埠輸出接收到的訂閱資料相關訊息，方便除錯
    Serial.print("read payload from mqtt broker:(");
    Serial.print(topic);  // 顯示 MQTT 主題
    Serial.print("/");
    Serial.print(ReciveBuff); // 顯示接收到的 JSON 字串
    Serial.print(")\n");

    // 解析接收到的 JSON 資料
    DeserializationError error = deserializeJson(doc, ReciveBuff);

    // 從 JSON 物件中取出各欄位數值
    const char* device = doc["Device"];      // 取得裝置識別碼，例如："48E729732158"
    float TT = doc["Temperature"];           // 取得溫度數值，例如：25.5
    float HH = doc["Humidity"];              // 取得濕度數值，例如：60.2

    // 將解析結果輸出到序列監視視窗
    Serial.print("Device: ");
    Serial.println(device);

    Serial.print("Temperature:");
    Serial.println(TT);
    Serial.print("Humidity:");
    Serial.println(HH);

    // 將解析出的資料顯示在 OLED 上
    showTitleonOled(MacData, 0);     // 第一列：顯示本機裝置 MAC
    showDeviceonOled(device, 2);     // 第二列：顯示 MQTT 來源裝置 MAC
    showTemperatureonOled(TT);       // 第三列：顯示溫度數值
    showHumidityonOled(HH);          // 第四列：顯示濕度數值
  }
}

// ===================== 系統初始化相關函式 =====================

// 函式名稱：initSensor()
// 功能：初始化所有感測模組與周邊裝置
void initSensor()
{
  initOled();    // 初始化 OLED 12864 顯示模組 (0.96吋 OLED BMD31M090)
  delay(2000);   // 延遲 2 秒，等待顯示模組穩定啟動
}

// 函式名稱：initAll()
// 功能：初始化整體系統，包含序列通訊與感測模組
void initAll()
{
  Serial.begin(9600);  // 啟動序列埠通訊，速率設定為 9600 bps
  Serial.println("System Start....."); // 印出系統啟動訊息
  initSensor();        // 呼叫 initSensor() 初始化感測模組
}

// 函式名稱：INITWIFI()
// 功能：初始化 WiFi 無線網路連線
void INITWIFI()  // 初始化 WiFi
{
  initWiFi();             // 連線到 WiFi 熱點
  ShowWiFiInformation();  // 顯示連線資訊（MAC / SSID / IP）
}

// ===================== WiFi 資訊顯示函式 =====================
// 函式名稱：ShowWiFiInformation()
// 功能：顯示目前 WiFi 連線相關資訊（MAC、SSID、IP）並儲存到全域變數
void ShowWiFiInformation()
{
  // 取得並儲存裝置的 MAC 位址
  MacData = GetMAC();   // 呼叫自訂函式 GetMAC()，取得目前裝置的 MAC Address
  
  Serial.println("---MAC Address----");  // 在序列埠輸出標題文字
  Serial.println(MacData);               // 印出裝置的 MAC Address
  Serial.println("");                    // 空一行，使顯示格式更清楚

  // 取得並儲存連線的 WiFi 熱點名稱
  Serial.println("---wifi access point----"); // 顯示目前使用的 WiFi AP 資訊標題
  SSIDData = GetSSID(); // 呼叫 GetSSID() 取得目前連線的 WiFi SSID
  
  Serial.println(SSIDData);  // 印出目前連線的 WiFi 熱點名稱 (SSID)

  // 取得並儲存 WiFi 分配的 IP 位址
  Serial.println("---Show IP Address----"); // 顯示 IP 位址標題
  IPData = GetIP();     // 呼叫自訂函式 GetIP() 取得目前 WiFi 分配到的 IP 位址
  
  Serial.println(IPData);  // 在序列埠輸出 IP 位址，例如：192.168.1.105
}

// =====將溫度數值格式化後顯示在OLED 顯示器之函式 =======
// 函式名稱：showTemperatureonOled()
// 參數：ss - 溫度數值（浮點數）
// 功能：將溫度數值格式化後顯示在 OLED 的指定行數
void showTemperatureonOled(float ss)
{
  // 組合字串："Temp:數值.C"，並顯示在 OLED 第 5 行（參數 4）
  showMsgonOled("Temp:" + String(ss) + ".C", 4);    
}

// =====將濕度數值格式化後顯示在OLED 顯示器之函式 =======
// 函式名稱：showHumidityonOled()
// 參數：ss - 濕度數值（浮點數）
// 功能：將濕度數值格式化後顯示在 OLED 的指定行數
void showHumidityonOled(float ss)
{
  // 組合字串："Humid:數值 %"，並顯示在 OLED 第 7 行（參數 6）
  showMsgonOled("Humid:" + String(ss) + " %", 6);  
}


