// ================================================================
// =============== 全域變數宣告區 (Global Variables) ===============
// ================================================================

// 宣告三個全域變數，用於儲存 WiFi 模組相關資訊
String MacData;   // 儲存目前裝置的 WiFi MAC Address（網路卡的唯一識別碼）
String SSIDData;  // 儲存目前連線的 WiFi 熱點名稱 (SSID)
String IPData;    // 儲存 WiFi 連線後由路由器分配的 IP 位址

// ------- 感測模組函式與外部函式引用宣告區 -----------
#include <String.h>    // Arduino 內建字串處理函式庫
#include "TCP.h"    //引用ESP-12F WiFi模組(BMCOM) BMC81M001自訂模組
#include "DHTLib.h"  // 自訂溫溼度模組函式庫
#include "OledLib.h"   // 自訂 OLED 顯示模組函式庫（提供 OLED 初始化、文字繪製、清屏等功能）
#include "clouding.h" //傳送感測資料到感測資料雲端代理人自訂模組

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統
void INITWIFI();                  // 初始化 WiFi 網路連線
void showTitleonOled(String ss,int row);   // 顯示標題文字於 OLED 第一列
void showIPonOled(String ss,int row);      // 顯示 IP 位址於 OLED
void showDeviceonOled(String ss,int row);  // 顯示裝置 Device ID 於 OLED
void ShowWiFiInformation();  //顯示wifi的基本參數

void setup() 
{
    initAll() ;
    delay(200);
    ShowDHTInformation();   //印出溫溼度感測器產品所有資訊 
    //-- WIFI Processing Code
    INITtWIFI();     // 初始化 WiFi 網路，並取得 SSID、IP 與 MAC 資料    
    //OLED Demo Code
    clearScreen()  ;  //清除螢幕
    // 畫BEST MODULE LOGO
    drawPicture(0,0,BestModule_LOGO,128, 64) ; //畫BEST MODULE LOGO
    delay(2000) ;
    clearScreen()  ;  //清除螢幕
    showTitleonOled("Temp & Humid SyS",0) ; //秀第一列字  
}

void loop() 
{
    // 從感測器讀取濕度數值並顯示
    HValue = readHumidity();        //讀取濕度數
    Serial.print("Humidity : ");
    Serial.print(HValue);       // 顯示濕度值
    Serial.print(" %    ");   

    // 從感測器讀取溫度數值並顯示
    TValue = readTemperature();        //讀取濕度數
    Serial.print("Temperature : ");
    Serial.print(TValue);    // 顯示溫度值
    Serial.println(" °C ");                 // 顯示溫度單位 °C
    showMsgonOled("Temp:"+String(TValue),2); //列印溫度於OLED上
    showMsgonOled("Humid:"+String(HValue),4);   //列印濕度於OLED上-   
    SendtoClouding() ;    //傳送感測資料到雲端
  delay(120000);//延遲120秒鐘
}

// 初始化所有感測模組
void initSensor()	// 初始化所有感測模組
{
  initOled();    // 初始化 OLED 12864 (0.96吋 OLED BMD31M090)
  delay(2000);   // 延遲 2 秒，等待顯示模組穩定
  initDHT();   // 初始化繼電器模組
  ShowDHTInformation();   //印出溫溼度感測器產品所有資訊
}

// ------------------ 系統初始化區 ------------------
// 初始化整體系統
void initAll()	// 初始化整體系統
{
  Serial.begin(9600);  // 啟動序列埠，速率 9600 bps
  Serial.println("System Start.....") ; //印出"System Start....." 
  initSensor(); 
}

// ---------- 無線網路初始化 --------
void INITWIFI()  //初始化 WiFi
{
  initWiFi();             // 連線 WiFi
  ShowWiFiInformation();  // 顯示 MAC / SSID / IP
}

// ---------顯示無線網路基本資訊---------
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





