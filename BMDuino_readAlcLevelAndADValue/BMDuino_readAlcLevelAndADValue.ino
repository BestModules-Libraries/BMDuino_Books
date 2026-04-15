/*************************************************
File:         BMDuino_readAlcLevelAndADValue
Description:  自動接收模組每秒輸出的資訊，
              並將酒精濃度與酒精A/D值印至序列埠監控視窗
Note:
**************************************************/
#include <BM22S3421-1.h>  // 引入酒精感測模組專用函式庫

uint8_t moduleInfo[14] = {0};  // 儲存模組回傳的資料封包（共14位元組）
uint16_t ADValue, AlcLevel, ADValueH, ADValueL; // 酒精A/D值、濃度值與高低位元暫存變數

// 初始化酒精感測器物件，使用軟體序列埠（STATUS腳位=8，RX=2，TX=3）
// BM22S3421_1 Alc(8, 2, 3); 
// 以下為其他硬體序列埠初始化範例（依據開發板與腳位選擇）：
// BM22S3421_1 Alc(8, &Serial);        // 使用硬體序列埠 Serial
BM22S3421_1 Alc(STATUS1, &Serial1); // BMduino-UNO 的 Serial1，STATUS腳位為22
// BM22S3421_1 Alc(STATUS2, &Serial2); // BMduino-UNO 的 Serial2，STATUS腳位為25

void setup()
{
  Alc.begin();         // 初始化序列埠（軟體或硬體），鮑率：9600，STATUS腳位設為輸入模式
  Serial.begin(9600);  // 初始化 Arduino 內建序列埠，鮑率：9600，用於監控輸出
  Serial.println("Module power on preheating...(about 3 mins)");
  preheatCountdown();  // 執行預熱倒數，等待感測器穩定（約3分鐘）
  Serial.println("End of preheating.");
  Serial.println();
  // alc.writeCommand(0xe0, 0x1e, AUTO_MODE); // 可設定模組為自動輸出模式（預設已是此模式）
}

void loop()
{
  // 檢查序列埠接收緩衝區是否有模組傳來的資料
  if (Alc.isInfoAvailable() == true) 
  {
    Alc.readInfoPackage(moduleInfo); // 讀取完整資料封包至 moduleInfo 陣列
    printInfo();                     // 呼叫函式，印出酒精濃度與A/D值
  }
}

/**
 * 印出感測器資訊：酒精濃度與酒精A/D值
 */
void printInfo()
{
  /* 印出酒精濃度（單位：mg/L） */
  Serial.print("Alc level: ");
  AlcLevel = moduleInfo[7];  // 資料封包第8位元組（索引7）為酒精濃度值
  Serial.println(AlcLevel);

  /* 印出酒精A/D值（類比數位轉換值） */
  Serial.print("Alc A/D Value: ");
  // 將第6位元組（高位）與第7位元組（低位）合併為16位元A/D值
  ADValue = ((uint16_t)moduleInfo[5] << 8) + moduleInfo[6];
  Serial.println(ADValue);
  Serial.println(); // 空一行以便閱讀
}

/**
 * 預熱倒數函式：等待感測器加熱穩定，約需3分鐘
 * 若模組已設定為自動輸出模式，則讀取封包中的倒數計時值；
 * 若為命令模式，則以 delay 方式倒數。
 */
void preheatCountdown()
{
  int16_t time = 180; // 預設預熱時間 180 秒（3分鐘）
  delay(1200);        // 初始延時，等待模組啟動

  if (Alc.isInfoAvailable() == true) // 如果模組已處於自動輸出模式
  {
    while (time > 0) 
    {
      if (Alc.isInfoAvailable() == true) // 持續檢查是否有新資料
      {
        Alc.readInfoPackage(moduleInfo);
        time = moduleInfo[10]; // 資料封包第11位元組（索引10）為剩餘預熱秒數
        Serial.print("time:");
        Serial.println(time);  // 印出剩餘預熱時間
      }
    }
  }
  else // 若模組處於命令模式，則以程式延遲模擬倒數
  {
    while (time > 0) 
    {
      time--;
      delay(1030); // 每約1.03秒減少1秒（略大於1秒以容許程式執行時間）
    }
  }
}