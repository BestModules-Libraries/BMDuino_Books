// ------- 全域變數宣告區 -----------
String MacData;   // 儲存 WiFi Mac Address Data
String SSIDData;   // 儲存 WiFi 熱點名稱 (SSID)
String IPData;     // 儲存 WiFi 分配到的 IP 位址
// MacData 在其他函式中宣告，作為全域變數

#include "TCP.h"       // TCP 通訊函式庫（包含 WiFi 初始化、MAC 取得等函式）
#include <String.h>    // Arduino 內建字串處理函式庫
#include "ConnecttoWebLib.h"       // TCP 通訊函式庫（包含 WiFi 初始化、MAC 取得等函式）

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統
void INITtWIFI();                  // 初始化 WiFi 網路連線
void ShowWiFiInformation();  //顯示wifi的基本參數


// ------------------ 初始化 ------------------
void setup()
{
  initAll();       // 初始化整體系統（啟動序列埠、初始化 OLED、Relay）
  delay(200);      // 延遲 200ms，確保硬體模組穩定
  INITtWIFI();     // 初始化 WiFi 網路，並取得 SSID、IP 與 MAC 資料

  //----------------------------
  Serial.println("Enter Loop()"); // 提示已經進入主迴圈 loop()
}

// ------------------ 主迴圈 ------------------
void loop() 
{    
    Connect2Web(dbagent) ; // 實作：連接到雲端網站
    delay(60000);
}

// ------------------ 系統初始化區 ------------------

// 初始化所有感測模組
void initSensor()
{
 
}

// 初始化整體系統
void initAll()
{
  Serial.begin(9600);  // 啟動序列埠，速率 9600 bps
  initSensor();        // 呼叫初始化感測模組
}

// ------------------ 初始化 WiFi -------------------------
void INITtWIFI()  //初始化 WiFi
{
  initWiFi();             // 連線 WiFi
  ShowWiFiInformation();  // 顯示 MAC / SSID / IP
}


// ----------------示 WiFi 資訊------------------
//**************************************************************
// 函式名稱：ShowWiFiInformation()
// 功能：顯示目前 WiFi 連線相關資訊（MAC、SSID、IP）
//**************************************************************
void ShowWiFiInformation()  // 顯示 WiFi 資訊
{
  MacData = GetMAC();   // 呼叫自訂函式 GetMAC()，取得目前裝置的 MAC Address
                        // 並將結果存入全域變數 MacData

  Serial.println("---MAC Address----");  // 在序列埠輸出標題文字
  Serial.println(MacData);               // 印出裝置的 MAC Address
  Serial.println("");                    // 空一行，使顯示格式更清楚

  Serial.println("---wifi access point----"); // 顯示目前使用的 WiFi AP 資訊標題
  SSIDData = GetSSID(); // 呼叫 GetSSID() 取得目前連線的 WiFi SSID
                        // 並儲存回全域變數 SSIDData

  Serial.println(SSIDData);  // 印出目前連線的 WiFi 熱點名稱 (SSID)

  Serial.println("---Show IP Address----"); // 顯示 IP 位址標題
  IPData = GetIP();     // 呼叫自訂函式 GetIP() 取得目前 WiFi 分配到的 IP 位址
                        // 並儲存回全域變數 IPData

  Serial.println(IPData);  // 在序列埠輸出 IPAddress，例如：192.168.1.105
}




