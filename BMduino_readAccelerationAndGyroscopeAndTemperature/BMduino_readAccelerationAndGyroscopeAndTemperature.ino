/*****************************************************************
File:         readAccelerationAndGyroscopeAndTemperature.ino
Description:  repeatedly obtain the 3 axis Acceleration(unit: g),
              3 axis Gyroscope(unit: °/s) and Temperature(unit: ℃) 
              through IIC and display the value in the serial port.
******************************************************************/

// 引入BMS56M605感測器的函式庫，這個函式庫包含了與該晶片通訊的方法。
#include "BMS56M605.h"

// 建立 BMS56M605 的物件實例，命名為 Mpu。
// 不同的建構子參數代表不同的硬體連接方式：

// 選項 1 (被註解掉): 使用預設的 I2C 介面 (通常是指 Wire) 並且將中斷腳位設定為 pin 8。
// BMS56M605 Mpu(8);//default select pin8 as intpin

// 選項 2 (目前啟用): 使用指定的 I2C 介面 (Wire1) 並且將中斷腳位設定為 pin 22。
// 這行告訴程式庫要透過哪個 I2C 匯流排與感測器通訊。
BMS56M605 Mpu(22,&Wire1);//default select pin22 as intpin

// 選項 3 (被註解掉): 使用指定的 I2C 介面 (Wire2) 並且將中斷腳位設定為 pin 25。
// BMS56M605 Mpu(25,&Wire2);//default select pin25 as intpin


// setup() 函數在 Arduino 板子啟動或重置時只會執行一次。
void setup() {
  // 初始化 BMS56M605 感測器。這個函數會設定感測器的初始狀態，
  // 例如檢查連線、設定感測器範圍等。
  Mpu.begin();

  // 啟動序列埠通訊，並設定通訊速率為 9600 baud。
  // 這讓我們可以將資料傳送到電腦上的序列埠監控視窗。
  Serial.begin(9600);
}

// loop() 函數在 setup() 執行完後會不斷地重複執行。
void loop() {
  // 從感測器讀取最新的數據。
  // getEvent() 函數會透過 I2C 通訊協定向 BMS56M605 晶片索取加速度、陀螺儀和溫度的數值，
  // 並將這些數值更新到 Mpu 物件的對應屬性中 (如 accX, gyroX, temperature)。
  Mpu.getEvent();

  // --- 顯示溫度數據 ---
  Serial.print("Temp = ");
  // Mpu.temperature 存放著最新的溫度值，單位為攝氏度 (℃)。
  Serial.print(Mpu.temperature);
  Serial.println(" ℃");

  // --- 顯示三軸加速度數據 ---
  Serial.print("ax = ")  ;
  // Mpu.accX, Mpu.accY, Mpu.accZ 存放著最新的三軸加速度值，單位為 g (地球重力加速度)。
  Serial.print(Mpu.accX);
  Serial.print("  ay = ");
  Serial.print(Mpu.accY);
  Serial.print("  az = ");
  Serial.print(Mpu.accZ);
  Serial.println("   g");

  // --- 顯示三軸陀螺儀數據 ---
  Serial.print("gx = ");
  // Mpu.gyroX, Mpu.gyroY, Mpu.gyroZ 存放著最新的三軸角速度值，單位為 °/s (度/秒)。
  Serial.print(Mpu.gyroX);
  Serial.print("  gy = ");
  Serial.print(Mpu.gyroY);
  Serial.print("  gz = ");
  Serial.print(Mpu.gyroZ);
  Serial.println(" °/s");

  // 列印一個空行，讓輸出看起來更整齊、更容易閱讀。
  Serial.println();

  // 延遲 1000 毫秒 (1 秒)。
  // 這會讓程式在這裡暫停一秒鐘，才進行下一次的讀取和輸出。
  // 這樣可以避免資料更新太快，讓使用者有時間閱讀序列埠監控視窗的內容。
  delay(1000);
}