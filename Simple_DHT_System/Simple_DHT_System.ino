// ------- 感測模組函式與外部函式引用宣告區 -----------
#include <String.h>    // Arduino 內建字串處理函式庫
#include "DHTLib.h"  // 自訂溫溼度模組函式庫
#include "OledLib.h"   // 自訂 OLED 顯示模組函式庫（提供 OLED 初始化、文字繪製、清屏等功能）

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統



void setup() 
{
    initAll() ;
    delay(200);
    ShowDHTInformation();   //印出溫溼度感測器產品所有資訊   
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

  delay(5000);//延遲五秒鐘
}

// 初始化所有感測模組
void initSensor()	// 初始化所有感測模組
{
  initOled();    // 初始化 OLED 12864 (0.96吋 OLED BMD31M090)
  delay(2000);   // 延遲 2 秒，等待顯示模組穩定
  initDHT();   // 初始化繼電器模組
  ShowDHTInformation();   //印出溫溼度感測器產品所有資訊
}

// 初始化整體系統
void initAll()	// 初始化整體系統
{
  Serial.begin(9600);  // 啟動序列埠，速率 9600 bps
  Serial.println("System Start.....") ; //印出"System Start....." 
  initSensor(); 
}






