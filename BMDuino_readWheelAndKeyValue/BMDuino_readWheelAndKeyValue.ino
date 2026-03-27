/*****************************************************************
檔案名稱: readWheelAndKeyValue.ino
功能描述:  將 BMK56T004 的 IIC 介面連接到 Arduino 的 Wire，
             同時將 INT 介面連接到 Arduino 的 PIN2。
             當按鍵被按下時，Arduino 將透過序列埠監視器輸出
             旋鈕與按鍵的狀態。
******************************************************************/

// 引入 BMK56T004 的函式庫，以便使用該模組的功能
#include "BMK56T004.h"

// 建立 BMK56T004 物件，並指定中斷腳位為 PIN2，使用 Wire 作為 I2C 通訊介面
// 參數說明：中斷腳位編號、I2C 通訊物件指標
BMK56T004 BMK56(2, &Wire); // 若使用 Arduino 標準 Wire，請使用此行

// 以下是針對不同 I2C 埠的備用設定，依開發板型號選擇開啟：
// BMK56T004 BMK56(22, &Wire1);  // 若使用 BMduino 的 Wire1，請取消註解此行
// BMK56T004 BMK56(25, &Wire2);  // 若使用 BMduino 的 Wire2，請取消註解此行

/**
 * 初始化函式，僅在 Arduino 啟動時執行一次
 * 用於設定序列埠通訊及初始化 BMK56T004 模組
 */
void setup() 
{
  // 啟動序列埠通訊，設定鮑率為 9600，用於與電腦進行資料傳輸
  Serial.begin(9600);
  
  // 初始化 BMK56T004 模組，包括設定 I2C 通訊及中斷腳位
  BMK56.begin();
}

/**
 * 主循環函式，Arduino 會重複執行此函式內的程式碼
 * 功能：持續檢查是否有按鍵觸發，若有則讀取並輸出旋鈕與按鍵數值
 */
void loop() 
{
  // 檢查 BMK56T004 的中斷腳位是否為低電位（表示有按鍵事件發生）
  // getINT() 返回值為 0 時，代表有按鍵被按下或旋鈕被轉動
  if (BMK56.getINT() == 0)
  {
      // 輸出旋鈕的當前數值
      Serial.print("wheelValue:");  // 顯示旋鈕數值標籤
      Serial.println(BMK56.readWheelValue()); // 讀取旋鈕數值並換行顯示

      // 輸出按鍵的狀態數值
      Serial.print("keyValue:");    // 顯示按鍵數值標籤
      Serial.println(BMK56.readKeyValue());   // 讀取按鍵數值並換行顯示
      
      // 注意：此處的 keyValue 可能代表按鍵編號或按壓狀態，依 BMK56T004 函式庫定義而定
  }
  // 若無中斷觸發，則跳過讀取，繼續循環，避免占用過多 CPU 資源
}