/*****************************************************************
檔案名稱: BMDuino_readStatusPin.ino
檔案功能: 讀取狀態引腳     
說明: LED會根據模組的不同狀態有不同的動作。
      a. 模組正常：標誌為0。無入侵警報。Led13亮起100毫秒。
      b. 模組警報：標誌為1，檢測到物體/人員通過，Led13持續亮起3秒。
      當偵測到模組狀態發生變化時，狀態資訊將會列印在序列埠監視器上。
******************************************************************/ 

#include "BM22S4221-1.h"  // 引入 PIR 感測器函式庫

uint8_t flag;              // 狀態標誌：0 表示正常，1 表示警報
uint8_t STATUS = 5;        // 狀態引腳設定為數位腳位 5

// 初始化 PIR 物件，使用軟體序列通訊（引腳 5, 6, 7）
// BM22S4221_1 PIR(STATUS, 6, 7); 

// 以下為其他硬體序列埠的初始化方式，目前已被註解掉
BM22S4221_1 PIR(22, &Serial1);  // 使用硬體 Serial1（需取消註解）
// BM22S4221_1 PIR(29, &Serial2);  // 使用硬體 Serial2（需取消註解）
// BM22S4221_1 PIR(STATUS, &Serial3); // 使用硬體 Serial3（需取消註解）
// BM22S4221_1 PIR(STATUS, &Serial4); // 使用硬體 Serial4（需取消註解）

void setup() {
  Serial.begin(9600);      // 初始化序列埠，用於除錯輸出
  PIR.begin();             // 初始化 PIR 感測器
  pinMode(STATUS, INPUT);  // 設定狀態引腳為輸入模式
  pinMode(13, OUTPUT);     // 設定板載 LED 引腳（13）為輸出模式
}

void loop() {
  // 狀況 1: 偵測到警報狀態（狀態引腳為 HIGH）且目前不是警報狀態
  if (PIR.getSTATUS() == HIGH && flag != 1) { 
    Serial.println("警報！有物體經過");  // 輸出警報訊息至序列埠
    flag = 1;  // 更新狀態標誌為警報狀態
  }

  // 狀況 2: 目前是警報狀態且狀態引腳變為 LOW（恢復正常）
  if (flag != 0 && PIR.getSTATUS() == LOW) {
    flag = 0;  // 更新狀態標誌為正常狀態
    Serial.println("模組正常；無警報");  // 輸出正常訊息至序列埠
  }

  // 根據狀態標誌控制 LED 行為
  switch (flag) {
    case 0:  // 正常狀態：LED 閃爍（亮 100ms，滅 900ms）
      digitalWrite(13, 1);  // 點亮 LED
      delay(100);           // 持續 100 毫秒
      digitalWrite(13, LOW); // 關閉 LED
      delay(900);           // 持續 900 毫秒
      break;

    case 1:  // 警報狀態：LED 持續亮起 1 秒（原註解為 3 秒，但程式碼中為 1000ms）
      digitalWrite(13, 1);  // 點亮 LED
      delay(1000);          // 持續 1000 毫秒
      break;
  }
}