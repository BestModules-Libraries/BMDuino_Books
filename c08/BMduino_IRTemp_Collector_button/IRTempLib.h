/********************* 程式整體運作解說 *************************/
/*
 * 【檔案名稱】IRTempLib.h
 * 【程式用途】
 *   本檔案為「BMH06203 紅外線溫度感測模組」的驅動與操作封裝，提供：
 *     1. 模組初始化（I2C 連線、啟動感測器）。
 *     2. 讀取物體溫度（Object Temperature）。
 *
 *   本模組常用於非接觸式測溫應用，例如：
 *     - BMduino_IRTemp_Collector_button.ino 中，以按鈕觸發讀取人體溫度。
 *
 * 【硬體與模組】
 *   - 感測器模組：BMH06203（紅外線溫度感測器）。
 *   - I2C 介面：本檔預設使用 Wire1；可依實際接線改用 Wire 或 Wire2
 *     （檔案中已保留對應註解行供切換）。
 *   - 量測目標：OBJ_TEMP（物體溫度），為 BMH06203 提供的常數，
 *     用以指定讀取「被測物體」的溫度而非感測器本身的環境溫度。
 *
 * 【溫度讀取說明】
 *   - mytherm.readTemperature(OBJ_TEMP)：
 *       向 BMH06203 請求讀取「物體溫度」，回傳值為 float（單位：°C）。
 *   - 回傳值儲存於全域變數 bodytemp，供主程式顯示與上傳使用。
 *
 * 【整體運作流程】
 *   ┌──────────────────────────┐
 *   │  initIRTemperature()     │  ← 初始化階段（由 initSensor() 呼叫）
 *   └───────────┬──────────────┘
 *               │
 *               ├─(1) mytherm.begin()：啟動感測器初始化程序，
 *               │       設定 I2C 位址並檢查模組是否存在。
 *               │
 *               └─(2) Serial.println("初始化紅外線模組")：印出提示訊息。
 *
 *   ┌──────────────────────────┐
 *   │  readIRTemperature()     │  ← 主程式呼叫時執行
 *   └───────────┬──────────────┘
 *               │
 *               └─ return mytherm.readTemperature(OBJ_TEMP)：
 *                      向感測器請求讀取物體溫度，並將 float 結果回傳。
 *
 *   ┌──────────────────────────┐
 *   │  主程式使用範例          │
 *   └───────────┬──────────────┘
 *               │
 *               └─ bodytemp = readIRTemperature();
 *                     將回傳的溫度存入全域變數 bodytemp，
 *                     後續用於 OLED 顯示與 SendtoClouding() 上傳。
 *
 * 【重要全域變數】
 *   - mytherm  ：BMH06203 物件（本檔預設綁定 Wire1）。
 *   - bodytemp ：體溫／物體溫度全域變數（float，單位 °C），
 *                由主程式 Oxygen 或 IRTemp Collector 使用。
 *
 * 【與其他模組的關係】
 *   - 本檔被 BMduino_IRTemp_Collector_button.ino 引用：
 *       * 於 initSensor() 中呼叫 initIRTemperature() 進行初始化。
 *       * 於 loop() 中呼叫 readIRTemperature() 取得溫度。
 *   - 底層依賴 BMH06203.h 提供的 mytherm 物件與其成員函式：
 *       begin()、readTemperature() 等。
 *
 * 【注意事項】
 *   - I2C 匯流排選擇（Wire / Wire1 / Wire2）需與實際硬體接線一致，
 *     否則 mytherm.begin() 會失敗（無法通訊）。
 *   - bodytemp 變數同時在本檔與主程式間共用，請避免重複宣告。
 *   - OBJ_TEMP 為 BMH06203.h 定義的常數，代表讀取物體溫度；
 *     若需讀取環境溫度，通常另有 AMB_TEMP 或類似常數可用。
 *   - readIRTemperature() 直接回傳感測器讀值，未做額外濾波或補償；
 *     若應用需要穩定度，可自行加入多次取樣平均或誤差補償。
 *   - 本檔為標頭檔（.h），實作直接寫在標頭中，若被多個 .ino/.cpp
 *     同時引用，可能造成重複定義問題；建議僅由單一主程式引用。
 */
// IR Temperature紅外線溫度模組自訂函式
#include <BMH06203.h>   // 引入 BMH06203 感測模組的函式庫，以便呼叫其 API 函式

//------------------- 模組初始化設定區 -------------------
// 下列三行只需保留一行依實際接線設定而定
// BMH06203 mytherm(&Wire);   // 若使用主 I2C 匯流排 (Wire)，請取消註解此行
BMH06203 mytherm(&Wire1);     // 若使用副 I2C 匯流排 (Wire1)，請取消註解此行
// BMH06203 mytherm(&Wire2);  // 若使用第三組 I2C 匯流排 (Wire2)，請取消註解此行
//--------------------------------------------------------

//-----準備全域變數-------------
float bodytemp = 0 ;  //體溫全域變數
//-----------------------------
// ------- 自定義函式宣告區 -----------
void initIRTemperature();    //初始化紅外線模組
float readIRTemperature(); //讀取紅外線模組取得量測溫度

//------自訂函式------
void initIRTemperature()    //初始化紅外線模組
{
  /* ===== 系統初始化階段 ===== */
  mytherm.begin();           // 啟動感測器的初始化程序，使其進入可通訊狀態 (設定 I2C 位址、檢查模組是否存在)
  Serial.println("初始化紅外線模組") ;
}

float readIRTemperature() //讀取紅外線模組取得量測溫度
{
    return mytherm.readTemperature(OBJ_TEMP);
    //回傳讀取紅外線模組取得量測溫度
}