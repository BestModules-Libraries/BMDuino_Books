/*************************************************
File:         BMDuino_readVOCLevelAndADValue
Description:  每秒接收模組自動輸出的資料，
              並將 VOC 等級與 VOC A/D 原始數值
              顯示於序列埠監控視窗
Note:
**************************************************/

// 引入 BM25S3421-1 VOC 感測模組的函式庫
#include <BM25S3421-1.h>

// 用來存放模組回傳的資料封包（共 14 Bytes）
uint8_t moduleInfo[14] = {0};

// 宣告 VOC 的 A/D 轉換值（原始感測值）與 VOC 等級
uint16_t ADValue, VOCLevel;

/*
 * 建立 VOC 感測模組物件
 * 參數說明（Software Serial 模式）：
 * 8  -> STATUS 腳位（模組狀態輸出）
 * 2  -> RX（Arduino 接收，模組 TX）
 * 3  -> TX（Arduino 傳送，模組 RX）
 */
//BM25S3421_1 VOC(8, 2, 3); // Software serial

// 以下為其他可選的建構方式（硬體序列埠）
// BM25S3421_1 VOC(8, &Serial);      // 使用 Serial
BM25S3421_1 VOC(STATUS1, &Serial1); // 使用 Serial1
// BM25S3421_1 VOC(STATUS2, &Serial2); // 使用 Serial2

void setup()
{
  // 初始化 VOC 模組
  // 1. 初始化軟體或硬體序列埠
  // 2. 設定鮑率為 9600 bps
  // 3. STATUS 腳位設為輸入模式
  VOC.begin();

  // 初始化 Arduino 與電腦通訊的序列埠
  Serial.begin(9600);

  // 顯示模組啟動與預熱提示
  Serial.println("Module power on  preheating...(about 3 mins)");

  // 執行模組預熱倒數（約 3 分鐘）
  preheatCountdown();

  Serial.println("End of preheating.");
  Serial.println();

  // 若需要可手動設定模組為自動輸出模式（預設即為 AUTO_MODE）
  // VOC.writeCommand(0xe0, 0x1e, AUTO_MODE);
}

void loop()
{
  /*
   * 檢查是否有來自 VOC 模組的新資料
   * isInfoAvailable() 會掃描序列埠接收緩衝區
   */
  if (VOC.isInfoAvailable() == true)
  {
    // 讀取一包完整的模組資料到 moduleInfo 陣列
    VOC.readInfoPackage(moduleInfo);

    // 解析並顯示感測資訊
    printInfo();
  }
}

void printInfo()
{
  /* 顯示 VOC 等級 */
  Serial.print("VOC level: ");

  // VOC 等級存放於 moduleInfo[7]
  VOCLevel = moduleInfo[7];
  Serial.println(VOCLevel);

  /* 顯示 VOC A/D 原始數值 */
  Serial.print("VOC A/D Value: ");

  /*
   * A/D 值為 16-bit 資料：
   * moduleInfo[5] 為高位元組
   * moduleInfo[6] 為低位元組
   * 需透過位移與加法組合
   */
  ADValue = ((uint16_t)moduleInfo[5] << 8) + moduleInfo[6];
  Serial.println(ADValue);

  Serial.println();
}

void preheatCountdown()
{
  // 模組預熱倒數時間（秒），初始為 180 秒
  int16_t time = 180;

  // 延遲約 1.2 秒，等待模組啟動穩定
  delay(1200);

  /*
   * 判斷是否能接收到模組資料
   * 若能接收 → AUTO_MODE
   * 若不能 → CMD_MODE
   */
  if (VOC.isInfoAvailable() == true)
  {
    // AUTO_MODE：模組會回傳剩餘預熱時間
    while (time > 0)
    {
      if (VOC.isInfoAvailable() == true)
      {
        // 讀取模組資料
        VOC.readInfoPackage(moduleInfo);

        // 剩餘預熱時間位於 moduleInfo[10]
        time = moduleInfo[10];

        // 顯示剩餘時間
        Serial.print("time:");
        Serial.println(time);
      }
    }
  }
  else
  {
    // CMD_MODE：模組未主動回傳時間，只能自行倒數
    while (time > 0)
    {
      time--;        // 每次遞減 1 秒
      delay(1030);   // 約 1 秒延遲
    }
  }
}
