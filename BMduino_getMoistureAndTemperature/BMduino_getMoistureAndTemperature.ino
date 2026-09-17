/*************************************************
File:             getMoistureAndTemperature.ino
Description:      開發板與單一土壤濕度感測器模組連接，
                  取得土壤濕度值與模組溫度值，
                  並透過序列埠監控視窗顯示。
Note:               
**************************************************/

// 匯入 BME34M101 土壤濕度感測器的函式庫
#include "BME34M101.h"

// 以下為三種不同的初始化方式，請根據使用的通訊埠選擇一種，並將其他註解掉：

// 使用軟體序列埠（腳位 5, 4）與感測器通訊
//BME34M101 mySoilMoistureSensor(5,4); // 若沒有使用 Serial，請保留此行

// 使用硬體序列埠 Serial1（適用於 BMduino 開發板）
BME34M101 mySoilMoistureSensor(&Serial1); // 若使用 Serial1 請取消註解

// 使用硬體序列埠 Serial2（適用於 BMduino 開發板）
//BME34M101 mySoilMoistureSensor(&Serial2); // 若使用 Serial2 請取消註解

void setup()
{
  // 初始化電腦端的序列通訊，鮑率設為 115200
  Serial.begin(115200);

  // 初始化感測器（設定通訊方式）
  mySoilMoistureSensor.begin();

  // 顯示等待連線訊息
  Serial.println("Check whether the module is connected,waiting...");

  // 持續檢查感測器是否已連線，若未連線則等待 1 秒後再檢查
  while (mySoilMoistureSensor.isConnected() == false)
  {
    delay(1000);
  }

  // 當感測器成功連線後顯示提示訊息
  Serial.println("Module is connecting.");
}

void loop()
{
  // 讀取並顯示土壤濕度值
  Serial.print("Get soil moisture value is: ");
  Serial.println(mySoilMoistureSensor.getMoisture()); // 取得濕度值
  delay(100); // 延遲 0.1 秒，避免讀取過於頻繁

  // 讀取並顯示感測器模組的溫度值
  Serial.print("Get module temperature value is: ");
  Serial.println(mySoilMoistureSensor.getTemperature()); // 取得溫度值
  delay(1000); // 延遲 1 秒後再進行下一次讀取
}