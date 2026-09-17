// ================================================================
// =============== 全域變數宣告區 (Global Variables) ===============
// ================================================================

#define RelayON   1   // 定義繼電器開啟狀態為 1
#define RelayOFF  0   // 定義繼電器關閉狀態為 0

uint8_t MaxRelay = 0;        // 儲存目前偵測到的繼電器模組數量
uint8_t relayStatus;         // 儲存單一繼電器的狀態（ON / OFF）
uint8_t Relaystatus[17];      // 儲存所有繼電器的狀態（陣列最多支援 9 個繼電器，依模組支援可達 16 顆）

//-----引入必要的函式庫--------
#include <BMP75M131.h> // 引入 BMP75M131 繼電器模組的函式庫，支援 I2C 介面控制

//-----------建立模組感測物件--------------
// BMP75M131 myRelay(&Wire);  // 若使用 Wire (默認 I2C 通道) 作為通訊接口，請取消註解此行
BMP75M131 myRelay(&Wire1);   // 使用 Wire1 (I2C 第二通道) 作為通訊接口（BMduino 上通常使用 Wire1）
 // BMP75M131 myRelay(&Wire2); // 若使用 Wire2 作為通訊接口，請取消註解此行
//-------------------------------------------------------------

// ------- 自定義函式宣告區 -----------
// 函式前置宣告 (Function Prototype)
void initRealy();             // 初始化繼電器模組
void TurnonRelay(int rno);    // 開啟第 rno 個繼電器
void TurnoffRelay(int rno);   // 關閉第 rno 個繼電器
void TurnonAllRelay();        // 開啟所有繼電器
void TurnoffAllRelay();       // 關閉所有繼電器
void GetAllRelayStatus();     // 取得所有繼電器狀態
int RelayStatus(int nn);      // 取得第 nn 個繼電器的狀態
//------需要使用OLEDLib.h宣告函式，-所以重複宣告一次------------
void showMsgonOled(String ss,int row); //列印Message於OLED上

// ------- 自定義函式實體程式區宣告抬頭 -----------
//-------------------------------------------------------------
// 初始化繼電器模組
//-------------------------------------------------------------
void initRealy()
{
  myRelay.begin();  // 初始化 BMP75M131 繼電器模組，啟動 I2C 通訊
  MaxRelay = myRelay.getNumber(); // 取得目前連接的繼電器數量
  Serial.print("Total Relay Amount is :(");
  Serial.print(MaxRelay);
  Serial.print(")\n"); // 輸出偵測到的繼電器數量
}

//-------------------------------------------------------------
// 開啟第 n 個繼電器
//-------------------------------------------------------------
void TurnonRelay(int rno) // 開啟第 n 個繼電器
{
  myRelay.setRelaySwitch(rno, RelayON); // 控制第 rno 個繼電器打開
  Serial.print("Relay(");
  Serial.print(rno);
  Serial.print("): is on\n"); // 輸出提示資訊
}

//-------------------------------------------------------------
// 關閉第 n 個繼電器
//-------------------------------------------------------------
void TurnoffRelay(int rno)  // 關閉第 n 個繼電器
{
  myRelay.setRelaySwitch(rno, RelayOFF); // 控制第 rno 個繼電器關閉
  Serial.print("Relay(");
  Serial.print(rno);
  Serial.print("): is off\n"); // 輸出提示資訊
}

//-------------------------------------------------------------
// 開啟所有繼電器
//-------------------------------------------------------------
void TurnonAllRelay()
{
  myRelay.setAllRelay(RelayON); // 將所有繼電器設定為開啟
}

//-------------------------------------------------------------
// 關閉所有繼電器
//-------------------------------------------------------------
void TurnoffAllRelay()
{
  myRelay.setAllRelay(RelayOFF); // 將所有繼電器設定為關閉
}

//-------------------------------------------------------------
// 取得所有繼電器狀態
//-------------------------------------------------------------
void GetAllRelayStatus()
{
  myRelay.getAllRelayStatus(Relaystatus); // 將所有繼電器狀態讀取到 Relaystatus 陣列中
}

//-------------------------------------------------------------
// 取得第n個繼電器狀態
//-------------------------------------------------------------
int RelayStatus(int nn)
{
  return (int)myRelay.getRelayStatus((uint8_t)nn); // 回傳第 nn 個繼電器狀態 (ON / OFF)
}

//-------------------------------------------------------------
// 列印繼電器號碼於OLED上
//-------------------------------------------------------------

void showRelayNOonOled(int ss) //列印繼電器號碼於OLED上
{
    showMsgonOled("Relay:("+String(ss)+")",4) ;//透過showMsgonOled()列印繼電器號碼於OLED上
 
}

//-------------------------------------------------------------
// 顯示命令在OLED上
//-------------------------------------------------------------

//---透過showMsgonOled()顯示命令在OLED上
void showCommandonOled(String ss)  //顯示命令在OLED上
{
  showMsgonOled(ss,6) ;//透過showMsgonOled()顯示命令在OLED上
}



