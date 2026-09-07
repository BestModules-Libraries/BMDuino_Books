/*************************************************
檔案名稱: BMduino_readDistance.ino
檔案描述: 1. 使用 Wire 函式庫與 BML36M001 進行 IIC 通訊。
          2. 使用硬體序列埠（鮑率 115200）與序列監控視窗通訊。
             當距離資料準備好時，讀取距離資料並顯示在序列監控視窗上。
連接方式： i2cPort:Wire  中斷腳位:D2
**************************************************/

// 引入 BML36M001 感測器的函式庫
#include "BML36M001.h"

// 建立 BML36M001 物件，使用中斷腳位 D2 與 Wire（I2C0）通訊
// BML36M001 BML36(2, &Wire);  // i2cPort, intPin，若不使用 HW Wire 請將此行註解

// 以下為其他 I2C 埠的設定範例，請依實際硬體需求啟用
BML36M001 BML36(22, &Wire1); // 若在 BMduino 上使用 HW Wire1，請取消註解此行
// BML36M001 BML36(25, &Wire2); // 若在 BMduino 上使用 HW Wire2，請取消註解此行

// 初始化函式，只在板子啟動或重置時執行一次
void setup(void)
{
  // 初始化序列通訊，設定鮑率為 9600，用於與電腦序列監控視窗通訊
  Serial.begin(9600);

  // 初始化 BML36M001 感測器，設定 I2C 通訊與中斷腳位
  BML36.begin();

  // 設定感測器的偵測模式為長距離模式
  // 偵測範圍：40 ~ 4000 毫米（最大可達 4000 毫米）
  BML36.setDistanceModeLong();

  // 啟動距離測量，感測器開始持續進行測距
  BML36.startRanging();
}

// 主迴圈函式，會不斷重複執行
void loop(void)
{
  // 檢查感測器的中斷腳位是否為高電位（資料是否已準備好）
  // 返回值：0 → 資料未準備好；1 → 資料已準備好
  if (BML36.getINT())
  {
    // 在序列監控視窗上輸出提示字串「Distance(mm):」，不換行
    Serial.print("Distance(mm):");

    // 讀取距離資料（單位：毫米），並換行顯示在序列監控視窗上
    Serial.println(BML36.readDistance());

    // 清除感測器的中斷標誌位，準備下一次測量
    BML36.clearInterrupt();
  }

  // 若無資料，則繼續迴圈等待，避免阻塞 CPU
}