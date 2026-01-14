
// ------- 感測模組函式與外部函式引用宣告區 -----------
#include <String.h>    // Arduino 內建字串處理函式庫
#include "OledLib.h"   // 自訂 OLED 顯示模組函式庫（提供 OLED 初始化、文字繪製、清屏等功能）

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統





void setup()
{
  initAll();       // 初始化整體系統（啟動序列埠、初始化 OLED、Relay）
  delay(200);      // 延遲 200ms，確保硬體模組穩定

   // 測試函式：顯示字串 "Hello World!" (字體大小 6x8)
  test_drawString_6x8();
  // 測試函式：顯示字串 "Hello World!" (字體大小 8x16)
  test_drawString_8x16();
  // 測試函式：顯示字串、字元和數字
  test_drawString_drawChar_drawNum();
  // 測試函式：繪製單個像素
  test_drawPixel();
  // 測試函式：繪製快速水平線和垂直線
  test_drawFastHLine_drawFastVLine();
  // 測試函式：繪製位圖（顯示 LOGO）
  test_drawBitmap();
  // 測試函式：顯示滾動功能，顯示不同方向的滾動效果
  test_variousScroll();
  // 測試函式：顯示翻轉顯示效果
  test_invertDisplay();
  // 測試函式：顯示調暗效果
  test_dim() ;
}

void loop()
{
  // 空的 loop，因為所有測試在 setup() 中完成
}


// 初始化所有感測模組
void initSensor()	// 初始化所有感測模組
{
  initOled();    // 初始化 OLED 12864 (0.96吋 OLED BMD31M090)
  delay(2000);   // 延遲 2 秒，等待顯示模組穩定
}

// 初始化整體系統
void initAll()	// 初始化整體系統
{
  Serial.begin(9600);  // 啟動序列埠，速率 9600 bps
  Serial.println("System Start.....") ; //印出"System Start....." 
  initSensor(); 
}




