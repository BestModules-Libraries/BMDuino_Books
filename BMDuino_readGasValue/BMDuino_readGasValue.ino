/*************************************************
File: BMDuino_readGasValue.ino
Description: 每秒自動接收模組輸出的資訊包，
並將部分資訊（氣體警報閾值、當前氣體濃度、氣體AD值）列印到序列埠監視器
Note:
**************************************************/

// 引入 BM22S3031-1 氣體感測器模組的函式庫
#include <BM22S3031-1.h>

// 定義儲存模組資訊包的陣列，長度為 34 個位元組
uint8_t moduleInfo[34] = { 0 };
// 定義變數：AD值、氣體濃度值、氣體警報閾值
uint16_t ADValue, gasValue, gasAlarmThreshold;

// 使用軟體序列埠初始化感測器物件：引腳 8 -> STATUS, 2 -> RX, 3 -> TX
// BM22S3031_1 gas(8, 2, 3);

// 以下是使用硬體序列埠的替代方案（目前被註解掉）
// BM22S3031_1 gas(8, &Serial); // 若在 BMduino 上使用 HW Serial，請取消註解此列
BM22S3031_1 gas(STATUS1, &Serial1); // 若在 BMduino 上使用 HW Serial1，請取消註解此列
// BM22S3031_1 gas(STATUS2, &Serial2); // 若在 BMduino 上使用 HW Serial2，請取消註解此列

void setup() {
  // 初始化軟體序列埠，鮑率設為 9600 bps，並將引腳 8 設為輸入模式
  gas.begin();

  // 初始化 Arduino 硬體序列埠（用於與電腦通訊），鮑率設為 9600 bps
  Serial.begin(9600);

  // 提示使用者模組正在預熱（約需 3 分鐘）
  Serial.println("Module preheating...(about 3 mins)");
  // 等待模組預熱完成（此函式會阻塞直到預熱結束）
  gas.preheatCountdown();
  Serial.println("End of module preheating.");
  Serial.println();
  delay(1200);  // 延遲 1.2 秒，確保系統穩定
}

void loop() {
  /* 掃描序列埠接收緩衝區，檢查是否有模組發送的資訊包可用 */
  if (gas.isInfoAvailable() == true) {
    // 讀取完整的資訊包到 moduleInfo 陣列中
    gas.readInfoPackage(moduleInfo);
    // 呼叫自訂函式，將部分資訊列印到序列埠監視器
    printInfo();
  }
}

void printInfo() {
  /* 列印氣體警報閾值（單位：PPM）*/
  Serial.print("Gas alarm threshold: ");
  // 資訊包中第 23 和 24 位元組組合為 16 位元的警報閾值（高位元組在前）
  gasAlarmThreshold = (moduleInfo[23] << 8) + moduleInfo[24];
  Serial.print(gasAlarmThreshold);
  Serial.println(" PPM");

  /* 列印當前氣體濃度（單位：PPM）*/
  Serial.print("Gas concentration: ");
  // 資訊包中第 9 和 10 位元組組合為 16 位元的氣體濃度值（高位元組在前）
  gasValue = (moduleInfo[9] << 8) + moduleInfo[10];
  Serial.print(gasValue);
  Serial.println(" PPM");

  /* 列印氣體 AD 值（原始類比數位轉換值）*/
  Serial.print("Gas AD Value: ");
  // 資訊包中第 5 和 6 位元組組合為 16 位元的 AD 值（高位元組在前）
  ADValue = (moduleInfo[5] << 8) + moduleInfo[6];
  Serial.println(ADValue);
  Serial.println();  // 列印空行，分隔每次輸出的資訊
}