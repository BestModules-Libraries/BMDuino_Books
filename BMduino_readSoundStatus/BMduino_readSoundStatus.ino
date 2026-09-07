/*****************************************************************
檔案：BMduino_readSoundStatus.ino 
描述：
  1. 聲音感測器（Sound Detector）是一塊整合麥克風與處理電路的小型板子。
     它不僅提供音訊輸出，還提供聲音存在的二進位指示、振幅的類比表示以及I2C通訊介面。
  2. 使用硬體序列埠（波特率115200）與序列埠監視器通訊。當有聲音時，
     讀取感測器資料並顯示在序列埠監視器上。
  3. 本專案介紹如何使用I2C介面。

注意：
  接線方法（聲音感測器：Arduino引腳）：
  GND → GND
  VDD → 3V3
  SDA → SDA1
  SCL → SCL1
  STA → STATUS1
******************************************************************/

#include <BMV23M001.h>  // 引入聲音感測器函式庫

// 根據使用的I2C介面選擇合適的初始化方式（僅啟用其中一行）

// BMV23M001 soundDetector(2,&Wire);   // 若使用標準Wire（I2C0），請取消註解此行（一般Arduino使用）
BMV23M001 soundDetector(22,&Wire1);  // 若在BMduino上使用Wire1（I2C1），請取消註解此行（範例中已啟用）
// BMV23M001 soundDetector(25,&Wire2); // 若在BMduino上使用Wire2（I2C2），請取消註解此行

void setup() 
{
  soundDetector.begin();   // 初始化聲音感測器，I2C通訊速率設定為100kHz
  Serial.begin(9600);    // 初始化序列埠通訊，波特率設為115200，用於偵錯訊息輸出
}

void loop()
{
  // 讀取聲音狀態，若回傳StatusFAIL表示通訊失敗
  if(soundDetector.readSoundStatus() == StatusFAIL)  
  {
    // 輸出錯誤訊息至序列埠監視器
    Serial.println("通訊失敗，請檢查接線！");
  }
  else  // 通訊成功，處理聲音狀態
  {
    Serial.print("聲音狀態: ");  // 輸出狀態標籤
    
    // 再次讀取狀態（此處為二進位狀態：有聲音/無聲音）
    if(soundDetector.readSoundStatus())  // 若回傳true（非零）表示有聲音
    {
      Serial.println("有聲音！");
    }
    else  // 回傳false（零）表示安靜
    { 
      Serial.println("安靜！");
    }
  }
  
  // 暫停10毫秒，避免序列埠輸出過於頻繁，並降低CPU使用率
  delay(10);
}