/*************************************************
檔案名稱：controlRelay.ino
功能描述：
1. 透過 I2C（Wire/Wire1/Wire2）介面控制 BMP75M131 繼電器模組。
2. 每秒切換一次繼電器（類似開關 LED）。
3. 並將狀態輸出到序列埠監控視窗（Serial Monitor）。
注意事項：
- 適用於使用 BMduino 平台與 BMP75M131 模組連接。
- 須依據實際接線選擇 Wire/Wire1/Wire2。

myRelay.begin()	初始化繼電器模組
setRelaySwitch(channel, state)	控制第 channel 號繼電器的開關，state = 1 為開啟，0 為關閉
getRelayStatus(channel)	讀取第 channel 號繼電器的目前狀態（回傳 0 或 1）

如果你有 多顆繼電器 模組（BMP75M131 支援多通道），可以使用 setRelaySwitch(2, 1) 控制第 2 號等等。

**************************************************/
//---------外部函式宣告區--------
#include <BMP75M131.h>  // 引入 BMP75M131 繼電器模組專用函式庫
//---------宣告變數--------
#define RelayOpen 1
#define RelayClose 0
#define doorlock 1

//---------全域變數--------
// 建立一個 BMP75M131 繼電器物件，指定使用 Wire1 I2C 通訊（適用於 BMduino）
// BMP75M131 myRelay(&Wire);   // 如果使用預設 I2C（Wire），取消這行註解
BMP75M131 myRelay(&Wire1);     // 如果使用 Wire1（例如 BMduino 上的第二組 I2C 腳位）
 // BMP75M131 myRelay(&Wire2); // 如果使用 Wire2，可啟用這行
//----------本程式函式宣告--------------------------------------
void initRelay();  // 初始化 BMP75M131 繼電器模組，啟用 I2C 通訊
uint8_t openDoor() ; //控制第 doorlock 號繼電器，設定為 RelayOpen（開啟）
uint8_t closeDoor() ;  // 控制第 doorlock 號繼電器，設定為 RelayClose（關閉）

//---------------------------------------------------------------------

void initRelay()  // 初始化 BMP75M131 繼電器模組，啟用 I2C 通訊
{
   myRelay.begin();          // 初始化 BMP75M131 繼電器模組，啟用 I2C 通訊
}

uint8_t openDoor()  //控制第 doorlock 號繼電器，設定為 RelayOpen（開啟）
{
    myRelay.setRelaySwitch(doorlock, RelayOpen);  // 控制第 doorlock 號繼電器，設定為 RelayOpen（開啟）
    return myRelay.getRelayStatus(doorlock);  // 取得第 doorlock 號繼電器的狀態
}

uint8_t closeDoor()   //// 控制第 doorlock 號繼電器，設定為 RelayClose（關閉）
{
    myRelay.setRelaySwitch(doorlock, RelayClose);  // 控制第 doorlock 號繼電器，設定為 RelayClose（關閉）
    return myRelay.getRelayStatus(doorlock);  // 取得第 doorlock 號繼電器的狀態
}



