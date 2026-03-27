/*************************************************
檔案: BMDuino_readCO2Concentration.ino
描述: 讀取二氧化碳濃度，單位：ppm (百萬分之一)
注意: 無
**************************************************/

// 引入 BM25S3321-1 二氧化碳感測器的函式庫
#include <BM25S3321-1.h>

// 定義狀態引腳 (STA_PIN) 為第 22 腳，用於感測器狀態輸入
#define STA_PIN 22 // 輸入引腳
// 以下兩行被註解掉，表示不使用軟體序列埠 (Software Serial)
// #define RX_PIN 2  // 模擬為 RX 引腳
// #define TX_PIN 3  // 模擬為 TX 引腳

// 宣告一個變數用來儲存讀取到的二氧化碳濃度值，初始值為 0
uint16_t CO2Value = 0;

// 以下為不同序列埠設定的範例（大部分被註解）：
// BM25S3321_1 CO2(STA_PIN, RX_PIN, TX_PIN); // 使用軟體序列埠：8->輸入引腳 2->RX, 3->TX

/* BMduino-UNO 硬體序列埠設定範例 */
// BM25S3321_1 CO2(STA_PIN, &Serial); // 使用硬體序列埠 Serial (通常用於電腦通訊)
BM25S3321_1 CO2(STA_PIN, &Serial1); // 使用硬體序列埠 Serial1
// BM25S3321_1 CO2(STA_PIN, &Serial2); // 使用硬體序列埠 Serial2
// BM25S3321_1 CO2(STA_PIN, &Serial3); // 使用硬體序列埠 Serial3
// BM25S3321_1 CO2(STA_PIN, &Serial4); // 使用硬體序列埠 Serial4

// 初始化設定函式，只在程式開始時執行一次
void setup()
{
  CO2.begin();        // 初始化二氧化碳感測器模組，鮑率設定為 9600 bps
  Serial.begin(9600); // 初始化 Arduino 序列埠，鮑率設定為 9600 bps，用於與電腦通訊

  // 在序列埠監控視窗顯示模組預熱訊息
  Serial.println("模組預熱中...(約需 60 秒)");
  CO2.preheatCountdown(); // 等待模組預熱完成（感測器需要時間穩定）
  Serial.println("模組預熱完成。");
  Serial.println(); // 空一行
  Serial.println("執行初始設定。");
  
  // 以下為可選的校正與設定功能（目前被註解）：
  // CO2.calibrateZeroPoint(); // 執行零點校正（需在乾淨空氣中進行）
  
  CO2.setRangeMax(5000); // 設定感測器的最大量測範圍為 5000 ppm
}

// 主迴圈函式，會重複不斷執行
void loop()
{
  // 讀取二氧化碳濃度值，並儲存到 CO2Value 變數中
  CO2Value = CO2.readCO2Value();
  
  // 透過序列埠將濃度值輸出到電腦的序列埠監控視窗
  Serial.print("二氧化碳濃度: ");
  Serial.print(CO2Value);
  Serial.println(" ppm");
  
  // 延遲 2 秒後再進行下一次讀取，避免過於頻繁讀取
  delay(2000);
}