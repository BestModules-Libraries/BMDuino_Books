/*************************************************
File:             controlRelay.ino
Description:      透過 IIC 控制繼電器模組，模擬 LED 開關，每秒切換一次。
Note:               
**************************************************/

// 匯入 BMduino 的繼電器控制函式庫
#include <BMP75M131.h>

// 宣告繼電器物件，並指定使用的 I2C 介面
//BMP75M131     myRelay(&Wire);   // 如果使用預設的 Wire（I2C）請取消註解這行
BMP75M131 myRelay(&Wire1);         // 若使用 BMduino 上的 Wire1（第二組 I2C），請取消註解這行
//BMP75M131     myRelay(&Wire2);   // 若使用 Wire2（第三組 I2C），請取消註解這行

void setup() {
  // 初始化序列埠通訊，設定鮑率為 9600，用於輸出除錯訊息
  Serial.begin(9600);

  // 初始化繼電器模組
  myRelay.begin();
}

void loop() 
{
  // 定義一個變數用來儲存繼電器的狀態
  uint8_t relyaStatus;

  // 設定第一個繼電器（通道 1）為開啟狀態（1 代表閉合，0 代表斷開）
  myRelay.setRelaySwitch(1, 1);
  Serial.println("Close the relay switch");  // 輸出訊息：繼電器閉合（導通）
  
  // 讀取第一個繼電器的當前狀態
  relyaStatus = myRelay.getRelayStatus(1);
  Serial.print("Get the relay switch status valus is: ");  // 輸出提示文字
  Serial.println(relyaStatus);  // 顯示繼電器狀態數值（應為 1）

  // 延遲 1 秒（1000 毫秒）
  delay(1000);

  // 設定第一個繼電器為關閉狀態
  myRelay.setRelaySwitch(1, 0);
  Serial.println("Open the relay switch");  // 輸出訊息：繼電器斷開
  
  // 再次讀取第一個繼電器的狀態
  relyaStatus = myRelay.getRelayStatus(1);
  Serial.print("Get the relay switch status valus is: ");  // 輸出提示文字
  Serial.println(relyaStatus);  // 顯示繼電器狀態數值（應為 0）

  // 延遲 1 秒後重複迴圈
  delay(1000);
}