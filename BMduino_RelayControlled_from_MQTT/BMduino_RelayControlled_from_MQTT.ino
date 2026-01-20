// ================================================================
// =============== 全域變數宣告區 (Global Variables) ===============
// ================================================================

// 宣告三個全域變數，用於儲存 WiFi 模組相關資訊
String MacData;   // 儲存目前裝置的 WiFi MAC Address（網路卡的唯一識別碼）
String SSIDData;  // 儲存目前連線的 WiFi 熱點名稱 (SSID)
String IPData;    // 儲存 WiFi 連線後由路由器分配的 IP 位址

//-----引入必要的函式庫--------
#include <String.h>    // Arduino 內建字串處理函式庫，提供字串操作功能
#include "commlib.h"   // 通訊相關的共用函式庫（可能包含封包處理、緩衝區管理等）
#include "OledLib.h"   // 自訂 OLED 顯示模組函式庫（提供 OLED 初始化、文字繪製、清屏等功能）
#include "RelayLib.h"  // 自訂 繼電器模組函式庫（提供 TurnonRelay、TurnoffRelay 等功能）
#include "TCP.h"       // TCP 通訊函式庫（包含 WiFi 初始化、MAC 取得等函式）
#include "MQTTLib.h"   // MQTT 函式庫，提供 MQTT 初始化、連線與發佈/訂閱訊息功能

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統（包含序列埠與感測模組）
void INITWIFI();                   // 初始化 WiFi 網路連線
void ShowWiFiInformation();        // 功能：顯示目前 WiFi 連線相關資訊（MAC、SSID、IP）並儲存到全域變數

// ------------------ 初始化函式 setup() ------------------
void setup()
{
  initAll();       // 初始化整體系統（啟動序列埠、初始化 OLED、Relay）
  delay(200);      // 延遲 200ms，確保硬體模組穩定運作
  INITWIFI();     // 初始化 WiFi 網路，並取得 SSID、IP 與 MAC 資料

  // 確認 WiFi 是否連線成功
  if (Wifi.getStatus())   
  {
    initMQTT();           // 初始化 MQTT 伺服器連線
    Serial.println("WIFI OK"); // 印出連線成功訊息至序列埠
  }

  //---------------------------------
  clearScreen();  // 清除 OLED 螢幕
  
  // 顯示 BEST MODULES 的 LOGO 圖片於 OLED 上
  drawPicture(0, 0, BestModule_LOGO, 128, 64); // 在座標 (0,0) 繪製 128x64 的 LOGO
  delay(3000);    // LOGO 顯示 3 秒
  clearScreen();  // 再次清除螢幕

  // 顯示系統資訊於 OLED 上
  showTitleonOled(MacData,0);  // 在 OLED 的第 0 行顯示 MAC 位址
  showIPonOled(IPData,2);      // 在 OLED 的第 2 行顯示 IP 位址
  
  //----------------------------
  Serial.println("Enter Loop()"); // 提示已經進入主迴圈 loop()
}

// ------------------ 主迴圈函式 loop() ------------------
void loop() 
{              
  // WiFi 斷線檢查與重連邏輯（目前註解掉）
  // 當 WiFi 斷線時可重新執行 INITtWIFI() 來重連

  // 從 MQTT Broker 接收資料
  // 讀取 MQTT Broker 下發的資料，並將資料存入 ReciveBuff、長度存入 ReciveBufflen、主題存入 topic
  Wifi.readIotData(&ReciveBuff, &ReciveBufflen, &topic);
  //{"Device":"083A8DF11676","RelayNumber":1,"Command":"ON"}


  // 如果接收到的資料長度不為 0，表示有收到資料
  if (ReciveBufflen)   
  {
    // 將接收到的 MQTT 主題與內容印出至序列埠
    Serial.print("read payload from mqtt broker:(");
    Serial.print(topic);   // 印出主題
    Serial.print("/");
    Serial.print(ReciveBuff); // 印出資料內容
    Serial.print(")\n");

    // 解析接收到的 JSON 格式資料
    DeserializationError error = deserializeJson(doc, ReciveBuff);

    // 從解析後的 JSON 物件中取出各欄位的數值
    const char* device     = doc["Device"];      // 取出裝置 ID，例: "48E729732158"
    int relaynumber        = doc["RelayNumber"]; // 取出繼電器編號，例: 2
    const char* command    = doc["Command"];     // 取出控制命令，例: "ON" 或 "OFF"

      // 將解析結果輸出到序列監視器，方便除錯
      Serial.print("Device: ");
      Serial.println(device);

      Serial.print("RelayNumber: ");
      Serial.println(relaynumber);

      Serial.print("Command:(");
      Serial.println(command);
      Serial.print(")\n");

    // 將解析結果顯示在 OLED 螢幕上
    showDeviceonOled(device,2);           // 在第 2 行顯示裝置 ID
    showRelayNOonOled(relaynumber);       // 顯示繼電器編號（行數可能由函式內部決定）
    showCommandonOled(command);           // 顯示命令字串（行數可能由函式內部決定）

    // 判斷接收到的命令並執行對應的繼電器操作
    if (String(command) == "ON")   // 若指令為 "ON"，開啟指定編號的繼電器
    {
      Serial.println("Turn on Relay");
      TurnonRelay(relaynumber); // 呼叫繼電器開啟函式
    }
    if (String(command) == "OFF")  // 若指令為 "OFF"，關閉指定編號的繼電器
    {
      Serial.println("Turn off Relay");
      TurnoffRelay(relaynumber); // 呼叫繼電器關閉函式
    }
  }
}

// ------------------ 系統初始化相關函式區 ------------------

// 函式名稱：initSensor()
// 功能：初始化所有感測模組與周邊裝置
void initSensor()
{
  initOled();    // 初始化 OLED 12864 顯示模組（使用 0.96吋 OLED BMD31M090）
  delay(2000);   // 延遲 2 秒，等待顯示模組穩定
  initRealy();   // 初始化繼電器模組
}

// 函式名稱：initAll()
// 功能：初始化整體系統，包含序列通訊與感測模組
void initAll()
{
  Serial.begin(9600);  // 啟動序列埠通訊，速率設定為 9600 bps（每秒位元數）
  Serial.println("System Start....."); // 印出系統啟動訊息至序列埠
  initSensor();        // 呼叫 initSensor() 初始化所有感測模組與周邊裝置
}

// 函式名稱：INITWIFI()
// 功能：初始化 WiFi 無線網路連線
void INITWIFI()
{
  initWiFi();             // 連線到預先設定的 WiFi 熱點
  ShowWiFiInformation();  // 顯示連線資訊（MAC / SSID / IP）並儲存至全域變數
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

  // 取得並儲存連線的 WiFi 熱點名稱（SSID）
  Serial.println("---wifi access point----"); // 顯示目前使用的 WiFi AP 資訊標題
  SSIDData = GetSSID(); // 呼叫 GetSSID() 取得目前連線的 WiFi SSID
  
  Serial.println(SSIDData);  // 印出目前連線的 WiFi 熱點名稱 (SSID)

  // 取得並儲存 WiFi 分配的 IP 位址
  Serial.println("---Show IP Address----"); // 顯示 IP 位址標題
  IPData = GetIP();     // 呼叫自訂函式 GetIP() 取得目前 WiFi 分配到的 IP 位址
  
  Serial.println(IPData);  // 在序列埠輸出 IP 位址，例如：192.168.1.105
}