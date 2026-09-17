/*************************************************
File:              readDataInfo
Description:      1. 使用 SoftwareSerial 介面 (鮑率 38400) 與 mySpo2 血氧模組通訊。
                  2. 使用硬體 Serial (鮑率 9600) 與序列埠監控視窗通訊，顯示量測結果。

程式運作解說：
                  本程式用於控制 BMH08002-4 血氧濃度感測模組 (mySpo2)。
                  首先在 setup() 中初始化序列埠與感測器，設定為「查詢回應模式」
                  (Mode 0x02)，並設定量測間隔為 300 毫秒。使用者需在 2 秒內
                  放置手指，隨後開始量測。

                  在 loop() 中，根據 Mode 值決定執行哪一種量測模式：
                  - Mode 1 (查詢模式)：呼叫 Mode_ask()，主動向模組要求量測資料。
                    若收到完整資料 (Status=0x02)，則輸出 SpO2、心率、PI 值，
                    並停止量測、進入睡眠模式。若手指移動或未放好，則顯示對應
                    提示訊息。
                  - 其他 Mode (連續計時模式)：呼叫 Mode_continuous_timing()，
                    持續檢查是否有新資料，若有則讀取 15 個位元組的原始資料包，
                    並將每個位元組的索引與數值印出。

注意事項：        根據使用的硬體序列埠 (UART) 或軟體序列埠，需選用對應的
                  建構子並註解/取消註解相關程式碼。
**************************************************/

#include <BMH08002-4.h>      // 引入 BMH08002-4 血氧感測器專用函式庫

// 建立 mySpo2 物件，使用軟體序列埠 (SoftwareSerial)，接腳定義：啟用腳位 2，接收腳位 5，傳送腳位 4
//BMH08002_4 mySpo2(2, 5, 4);   // 若使用軟體序列埠，請保留此行，並註解掉下方硬體序列埠的寫法

// 若使用 BMduino 的硬體序列埠，請取消註解以下其中一行，並註解掉上方軟體序列埠的寫法
 BMH08002_4 mySpo2(22, &Serial1);  // 使用 Serial1 (硬體序列埠1)
// BMH08002_4 mySpo2(25, &Serial2);  // 使用 Serial2 (硬體序列埠2)
// BMH08002_4 mySpo2(2, &Serial3);   // 使用 Serial3 (硬體序列埠3)
// BMH08002_4 mySpo2(2, &Serial4);   // 使用 Serial4 (硬體序列埠4)

uint8_t Mode = 0;          // 儲存目前的工作模式 (0: 連續計時模式, 1: 查詢模式)
uint8_t rBuf[15] = {0};    // 用來存放從感測器讀取到的 15 個位元組資料包
uint8_t Status = 0;        // 儲存查詢回應的狀態碼
uint8_t flag = 0;          // 輔助旗標，用於避免重複顯示相同的手指狀態訊息

void setup()
{
  Serial.begin(9600);      // 初始化硬體序列埠 (與電腦監控視窗通訊)，鮑率 9600
  mySpo2.begin();          // 初始化血氧感測器模組 (內部序列埠鮑率為 38400)

  // 設定感測器為「查詢回應模式」(0x02)，且偵測到手指時亮紅燈
  mySpo2.setModeConfig(0x02);

  // 設定量測時間間隔為 300 毫秒 (影響資料更新頻率)
  mySpo2.setTimeInterval(300);

  Serial.println("請放置您的手指");   // 提示使用者將手指放上感測器
  delay(2000);                       // 等待 2 秒，讓使用者有時間放置手指

  mySpo2.beginMeasure();             // 開始進行量測

  // 讀取目前設定的模式組態
  Mode = mySpo2.getModeConfig();

  // 若模式為 0x02 (查詢回應) 或 0x03 (查詢+連續混合)，則將 Mode 設為 1 (查詢模式)
  if (Mode == 0x02 || Mode == 0x03)
  {
    Mode = 1;
  }
  else
  {
    Mode = 0;   // 否則設為 0 (連續計時模式)
  }
}

void loop()
{
  // 根據 Mode 值執行對應的量測處理函式
  switch (Mode)
  {
    case 1:
      Mode_ask();               // 查詢模式：主動要求資料
      break;
    default:
      Mode_continuous_timing(); // 連續計時模式：被動接收資料
  }
}

// ============================================================
// 函式名稱：Mode_ask
// 功能說明：適用於「查詢回應模式」(Mode 0x02 或 0x03)
//           主動向感測器要求一筆量測資料包，並根據回應狀態
//           顯示量測結果或提示使用者調整手指位置。
// ============================================================
void Mode_ask()
{
  // 向感測器要求資料包，並將結果存入 rBuf，回傳狀態碼
  Status = mySpo2.requestInfoPackage(rBuf);

  if (Status == 0x02)   // 狀態 0x02 表示量測完成，資料有效
  {
    Serial.println("量測完成，可移除手指");
    Serial.print("血氧濃度 (SpO2): ");
    Serial.print(rBuf[0], DEC);   // rBuf[0] 為 SpO2 數值 (%)
    Serial.println("%");
    Serial.print("心率 (Heart rate): ");
    Serial.print(rBuf[1], DEC);   // rBuf[1] 為心率 (BPM)
    Serial.println("BPM");
    Serial.print("灌注指數 (PI): ");
    Serial.print((float)rBuf[2] / 10); // rBuf[2] 為 PI 值，需除以 10 還原
    Serial.println("%");

    mySpo2.endMeasure();   // 停止量測
    mySpo2.sleep();        // 讓模組進入休眠 (省電)
  }

  if (Status == 0x01 && flag != 1)   // 狀態 0x01 表示手指移動中，訊號不穩
  {
    Serial.println("請勿移動手指");
    flag = 1;   // 設定旗標，避免重複顯示
  }

  if (Status == 0x00 && flag != 0)   // 狀態 0x00 表示手指未正確放置或已移除
  {
    Serial.println("請重新放置手指");
    flag = 0;   // 清除旗標，以便下次重新提示
  }
}

// ============================================================
// 函式名稱：Mode_continuous_timing
// 功能說明：適用於「連續計時模式」(非查詢模式)
//           持續檢查序列埠緩衝區是否有新資料，若有則讀取
//           完整的 15 位元組資料包，並將每個位元組的索引
//           與數值印出至序列監控視窗。
// ============================================================
void Mode_continuous_timing()
{
  // 檢查是否有新的量測資料可讀取
  if (mySpo2.isInfoAvailable() == true)
  {
    // 讀取一筆完整的 15 位元組資料包到 rBuf
    mySpo2.readInfoPackage(rBuf);

    // 將每個位元組的索引 (0~14) 與其數值印出
    for (uint8_t i = 0; i < 15; i++)
    {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(rBuf[i]);
    }
  }
}