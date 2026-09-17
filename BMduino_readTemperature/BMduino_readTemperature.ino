/*****************************************************************
File:         readTemperature.ino
Description:  1. 使用 I2C（100k）與 BMH63K203 感測器通訊。
              2. 使用硬體序列埠（BAUDRATE 9600）與序列埠監控視窗通訊。            
Note:
******************************************************************/

// 匯入 BMH06203 函式庫，該函式庫用於控制 BMH06203 溫度感測器
#include <BMH06203.h>

// 宣告 BMH06203 物件 mytherm，並指定使用 Wire1 進行 I2C 通訊
// 若您使用的是 BMduino 上的 Wire，請取消註解下方第一行
// 若您使用的是 BMduino 上的 Wire1，請取消註解下方第二行
// 若您使用的是 BMduino 上的 Wire2，請取消註解下方第三行
//BMH06203 mytherm(&Wire); // 使用 Wire（通常為預設 I2C）
BMH06203 mytherm(&Wire1);   // 使用 Wire1（第二組 I2C）
// BMH06203 mytherm(&Wire2); // 使用 Wire2（第三組 I2C）

void setup() 
{
  /* 初始化感測器，設定為 I2C 模式 */
  mytherm.begin();      

  /* 初始化序列通訊，設定傳輸速率為 9600 bps，用於輸出資料到序列埠監控視窗 */
  Serial.begin(9600);     
}

void loop()
{
  // 讀取並輸出物體溫度（OBJ_TEMP）
  Serial.print("The OBJTemp is:");                    // 輸出提示文字
  Serial.print(mytherm.readTemperature(OBJ_TEMP));    // 讀取物體溫度並輸出
  Serial.println("℃");                                // 輸出溫度單位（攝氏）
}