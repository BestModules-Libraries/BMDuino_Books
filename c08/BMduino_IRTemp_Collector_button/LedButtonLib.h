/********************* 程式整體運作解說 *************************/
/*
 * 【檔案名稱】LedButtonLib.h
 * 【程式用途】
 *   本檔案為「BMK22M131 LED 按鈕模組」的驅動與操作封裝，提供：
 *     1. 外部中斷（Interrupt）機制，即時偵測按鈕事件。
 *     2. 模組初始化（I2C 連線、LED 模式、長按時間設定）。
 *     3. 按鈕狀態查詢函式（單顆 / 全部）。
 *
 *   本模組常用於需要「按鈕觸發」的應用場景，例如：
 *     - BMduino_IRTemp_Collector_button.ino 中，以按鈕觸發紅外線溫度讀取。
 *
 * 【硬體與模組】
 *   - 按鈕模組：BMK22M131（LED + 按鈕複合模組，支援多顆串接）。
 *   - I2C 介面：本檔預設使用 Wire1；可依實際接線改用 Wire 或 Wire2
 *     （檔案中已保留對應註解行供切換）。
 *   - 外部中斷腳位：預設 D22（BMCOM1），可依需求改用 D25（BMCOM2）。
 *   - 最多支援 16 顆按鈕串接（BTstatus[17] 陣列，索引 1~16）。
 *
 * 【按鈕狀態碼定義】
 *   ┌────────┬──────────────────────┐
 *   │  狀態碼 │  意義                │
 *   ├────────┼──────────────────────┤
 *   │  0x00  │  無動作／未按下      │
 *   │  0x01  │  短按（Short Press） │
 *   │  0x02  │  長按（Long Press）  │
 *   └────────┴──────────────────────┘
 *
 * 【長按判斷】
 *   - 巨集 longpressed 定義為 500（毫秒）。
 *   - 透過 myButton.setButtonLongOnTime() 對每顆按鈕設定長按門檻；
 *     按鈕持續按下的時間超過 500ms 即判定為「長按」。
 *
 * 【整體運作流程】
 *   ┌──────────────────────┐
 *   │  intLedButton()      │  ← 初始化階段（由 initSensor() 呼叫）
 *   └──────────┬───────────┘
 *              │
 *              ├─(1) attachInterrupt()：設定外部中斷，
 *              │       偵測 intPin 下降沿（FALLING）時執行 ButtonInt()。
 *              │
 *              ├─(2) myButton.begin()：初始化 BMK22M131（I2C）。
 *              │
 *              ├─(3) myButton.ledButtonMode(1)：啟用 LED 按鈕模式。
 *              │
 *              ├─(4) myButton.getNumber()：取得串接的按鈕總數（MaxButton）。
 *              │
 *              ├─(5) myButton.isConnected()：確認模組連線，
 *              │       並透過 Serial 印出連線狀態與按鈕總數。
 *              │
 *              └─(6) 迴圈設定每顆按鈕的長按時間為 longpressed（500ms）。
 *
 *   ┌──────────────────────┐
 *   │  按鈕被按下時        │
 *   └──────────┬───────────┘
 *              │
 *              └─ 觸發外部中斷 → ButtonInt() 將 int_flag 設為 1，
 *                 通知主程式有按鈕事件待處理。
 *
 *   ┌──────────────────────┐
 *   │  主程式呼叫查詢函式  │
 *   └──────────┬───────────┘
 *              │
 *              ├─ getButton(btn) / getButtonStatus(btn)：
 *              │     檢查 int_flag；若為 1 則重設為 0，
 *              │     並向模組讀取第 btn 顆按鈕的狀態碼回傳。
 *              │     若 btn > MaxButton，直接回傳 0x00（未按下）。
 *              │
 *              └─ getAllButton()：
 *                    檢查 int_flag；若為 1 則重設為 0，
 *                    讀取全部按鈕狀態至 BTstatus[]，
 *                    串接成字串後回傳。
 *
 * 【重要全域變數】
 *   - myButton    ：BMK22M131 物件（本檔預設綁定 Wire1）。
 *   - int_flag    ：中斷旗標（0=無事件、1=有事件待處理）。
 *   - MaxButton   ：目前偵測到的 LED 按鈕模組總數。
 *   - buttonStatus：單一按鈕狀態暫存（由 getAllButton 使用）。
 *   - BTstatus[17]：所有按鈕狀態陣列（索引 1~16 對應第 1~16 顆）。
 *
 * 【巨集定義】
 *   - longpressed ：長按判斷時間（500 毫秒）。
 *   - intPin      ：外部中斷輸入腳位（預設 D22）。
 *
 * 【與其他模組的關係】
 *   - 本檔被 BMduino_IRTemp_Collector_button.ino 引用，
 *     於 initSensor() 中呼叫 intLedButton() 進行初始化，
 *     並於 loop() 中以 getButton(1) 查詢第一顆按鈕狀態。
 *   - 底層依賴 BMK22M131.h 提供的 myButton 物件與其成員函式：
 *       begin()、ledButtonMode()、getNumber()、isConnected()、
 *       setButtonLongOnTime()、getButtonStatus() 等。
 *
 * 【注意事項】
 *   - 本檔使用「外部中斷 + 旗標」機制：中斷服務函式僅設旗標，
 *     實際讀取 I2C 資料在主迴圈中執行，避免在 ISR 中進行耗時操作。
 *   - getButton() 與 getButtonStatus() 兩者功能完全相同，
 *     皆為讀取第 btn 顆按鈕狀態；命名差異可能為歷史因素，
 *     實際使用時可擇一即可。
 *   - 這兩個函式在 int_flag 為 0 時「沒有回傳值」，
 *     屬於潛在的程式缺陷；若需穩定行為，建議補上 else 分支
 *     回傳 0x00，避免回傳未定義值。
 *   - BTstatus 陣列索引從 1 開始（對應第 1 顆按鈕），
 *     使用時請注意索引範圍為 1~MaxButton。
 *   - 請依實際硬體接線選擇 Wire / Wire1 / Wire2 與中斷腳位。
 */
//------------------------------------------------------
#include <Wire.h>        // 引入 I2C（TWI）通訊的標準函式庫，支援多設備傳輸
#include <BMK22M131.h>   // 引入 BMK22M131 按鈕模組的函式庫，用於 LED+按鈕模組控制
//------------------------------------------------------
#define longpressed 500   // 定義「長按」的時間長度 (單位：500毫秒)，超過此值視為長按
#define intPin 22         // 設定外部中斷輸入腳位，使用 BMCOM1 (腳位 D22)
//#define intPin 25       // 如果使用 BMCOM2，則改為腳位 D25

// 建立 BMK22M131 類別物件 myButton，並指定 I2C 通道
// 可依實際使用的 Wire, Wire1, Wire2 來決定使用哪組 I2C 腳位
//BMK22M131 myButton(intPin, &Wire);
BMK22M131 myButton(intPin, &Wire1);
//BMK22M131 myButton(intPin, &Wire2);

//------------------------------------------------------

// 宣告中斷旗標，預設為 0
// 當發生中斷時會設為 1，提醒主程式需要處理
uint8_t int_flag = 0;

uint8_t MaxButton = 0;   // 用來儲存目前偵測到的 LED 按鈕模組數量
uint8_t buttonStatus;    // 用來儲存單一按鈕的狀態（短按 / 長按）
uint8_t BTstatus[17];    // 用來存放所有聯集（串接）按鈕的狀態，最多支援 16 顆按鈕

//------------------ 函式宣告區 ------------------//
void ButtonInt();                   // 外部中斷觸發後執行的函數
void intLedButton();                // 初始化 LED 按鈕模組
String getAllButtonStatus();        // 取得所有按鈕的即時狀態
uint8_t getButton(uint8_t btn);  //回傳第btn按鈕的狀態碼
uint8_t getButtonStatus(uint8_t btn);   //回傳第btn按鈕的狀態碼

//================================================//
// 函式區
//================================================//

//-------------------------
// 外部中斷服務函數（ISR）
// 當指定腳位偵測到下降沿 (FALLING edge) 時被呼叫
//-------------------------
void ButtonInt() 
{
  int_flag = 1;  // 將中斷旗標設為 1，通知主程式有按鈕事件需要處理
}

//-------------------------
// 初始化 LED 按鈕模組
// 功能：
//   1. 啟用外部中斷偵測
//   2. 初始化 I2C 與模組
//   3. 開啟 LED 模式
//   4. 設定「長按」的判斷時間
//-------------------------
void intLedButton()   
{
  // 設定外部中斷，偵測到「下降沿」時執行 ButtonInt()
  attachInterrupt(digitalPinToInterrupt(intPin), ButtonInt, FALLING);

  // 初始化 BMK22M131 按鈕模組（I2C 設定）
  myButton.begin();

  // 啟用 LED 按鈕模式（參數 1 表示開啟 LED）
  myButton.ledButtonMode(1);

  // 取得目前連接的 LED 按鈕模組數量
  MaxButton = myButton.getNumber();

  // 確認模組是否成功連線
  Serial.println("Check whether the module is connected, waiting...");
  if (myButton.isConnected() == true) 
  {
    Serial.println("The module is connecting");  
    Serial.print("Total Amount of LEDButton is Connected:(");  
    Serial.print(MaxButton);  
    Serial.print(")\n");  
  } 

  // 設定每一顆按鈕的「長按時間」
  for (int i=0 ; i <MaxButton ; i++)  
  {
      myButton.setButtonLongOnTime((uint8_t)i, longpressed);  
  }
}

//-------------------------
// 取得所有聯集（串接）按鈕的狀態
// 回傳一個字串，內容為每個按鈕的狀態碼
//-------------------------
String getAllButton() 
{
  String tmp="";   // 用來累加所有按鈕狀態的字串

  // 當中斷旗標被設定為 1（表示有按鈕事件發生）
  if (int_flag) 
  {
    int_flag = 0;  // 重設旗標，避免重複觸發

    // 從模組讀取所有按鈕的狀態，存入 BTstatus 陣列
    // BTstatus[i] 代表第 i 顆按鈕的狀態
    // 狀態可能為：0=無動作，1=短按，2=長按
    buttonStatus = myButton.getButtonStatus(BTstatus);

    // 將所有按鈕狀態轉成字串並累加
    for(int i=1 ; i <= MaxButton; i++)  
    {
      tmp.concat(BTstatus[i]);  
    }
  }  

  // 將按鈕狀態輸出到序列埠監控視窗
  Serial.print("All Button:(") ;
  Serial.print(tmp) ;
  Serial.print(")\n") ;

  return tmp;   // 回傳所有按鈕狀態的字串
}


uint8_t getButtonStatus(uint8_t btn)  //回傳第btn按鈕的狀態碼
{
  if (btn >MaxButton) //檢查超出所有按鈕數目
  {
      return (0x00) ; //0x00：未按下
  }
  else 
 {
   // 當中斷旗標被設定為 1（表示有按鈕事件發生）
  if (int_flag)   //表示有按鈕事件發生
  {
    int_flag = 0;  // 重設旗標，避免重複觸發

    // 從模組讀取getButtonStatus(btn)按鈕的狀態，回傳資料
    // getButtonStatus(btn)按鈕的狀態
    // 狀態可能為：0=無動作，1=短按，2=長按
    return myButton.getButtonStatus(btn);
    }  
  }
}

uint8_t getButton(uint8_t btn)  //回傳第btn按鈕的狀態碼
{
  if (btn >MaxButton) //檢查超出所有按鈕數目
  {
      return (0x00) ; //0x00：未按下
  }
  else 
 {
   // 當中斷旗標被設定為 1（表示有按鈕事件發生）
  if (int_flag)   //表示有按鈕事件發生
  {
    int_flag = 0;  // 重設旗標，避免重複觸發

    // 從模組讀取getButtonStatus(btn)按鈕的狀態，回傳資料
    // getButtonStatus(btn)按鈕的狀態
    // 狀態可能為：0=無動作，1=短按，2=長按
    return myButton.getButtonStatus(btn);
    }  
  }
}