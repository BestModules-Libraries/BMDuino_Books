/*************************************************
File:             getButtonStatus.ino
Description:      本範例為開發板連接按鈕模組的應用，
                  當接收到中斷訊號時，會透過 I2C 通訊方式
                  讀取按鈕的觸發狀態（短按或長按）。
Note:             
**************************************************/

// 引入 I2C（TWI）通訊的標準函式庫，用於與週邊模組進行通訊
#include <Wire.h>
// 引入 BMK22M131 按鈕模組的專用函式庫，提供控制該模組的函式
#include <BMK22M131.h>

// 定義中斷腳位：使用 BMCOM1 時對應的腳位為 D22
#define intPin 22
// 註解掉的選項是其他通訊埠的腳位定義，可根據實際硬體連接選擇
//#define intPin 25   //for BMCOM2

// 建立一個 BMK22M131 類別的物件 myButton，並指定使用中斷腳位與 I2C 介面
// 可根據實際使用的 I2C 通道選擇 Wire、Wire1 或 Wire2
//BMK22M131 myButton(intPin, &Wire);
BMK22M131 myButton(intPin, &Wire1);  // 目前使用 Wire1 作為 I2C 通訊埠
//BMK22M131 myButton(intPin, &Wire2);

// 宣告一個中斷觸發旗標，用於在主程式中判斷是否有中斷事件發生
uint8_t int_flag = 0;

// 宣告一個變數用來儲存讀取到的按鈕狀態（短按或長按）
uint8_t buttonStatus;

//------------------
// 函式原型宣告：外部中斷服務函數，當中斷腳位偵測到下降沿時會自動呼叫此函式
void ButtonInt();

// 初始化設定函式，在 Arduino 啟動時只會執行一次
void setup() {
  // 啟動序列埠通訊，設定傳輸速率為 9600 bps，用於與電腦進行除錯訊息傳輸
  Serial.begin(9600);

  // 設定中斷腳位功能：
  // 1. 將數位腳位 intPin 設定為外部中斷輸入腳
  // 2. 當腳位電位由高電平變為低電平（下降沿）時觸發中斷
  // 3. 觸發中斷時會自動呼叫 ButtonInt() 函式
  attachInterrupt(digitalPinToInterrupt(intPin), ButtonInt, FALLING);

  // 啟動按鈕模組，初始化 I2C 通訊並進行模組的基本設定
  myButton.begin();

  // 啟用按鈕模組的 LED 顯示模式，參數 1 表示開啟 LED 指示功能
  myButton.ledButtonMode(1);

  // 檢查按鈕模組是否成功透過 I2C 連線
  Serial.println("Check whether the module is connected, waiting...");
  if (myButton.isConnected() == true) 
  {
    Serial.println("The module is connecting");  // 顯示模組連線成功
  } // end of  if (myButton.isConnected() == true) 
}

// 主迴圈函式，Arduino 啟動後會不斷重複執行此函式
void loop() 
{
  // 檢查中斷旗標是否為 1，表示有按鈕中斷事件發生
  if (int_flag) 
  {
    int_flag = 0;  // 重設旗標為 0，避免重複處理同一次中斷事件

    // 透過 I2C 讀取按鈕模組的觸發狀態，參數 1 表示讀取第一個按鈕通道（或按鈕 ID）
    buttonStatus = myButton.getButtonStatus(1);

    // 根據讀取到的狀態值判斷按鈕觸發類型
    if (buttonStatus == 1) 
    {
      // 輸出按鈕狀態值與短按訊息
      Serial.print("Button Status:(") ;
      Serial.print(buttonStatus) ;
      Serial.print(")\n") ;
      Serial.println("The button is short pressed.");  // 短按觸發
    } // end of  if (buttonStatus == 1) 
    else 
    {
      // 輸出按鈕狀態值與長按訊息（假設非 1 即為長按）
      Serial.print("Button Status:(") ;
      Serial.print(buttonStatus) ;
      Serial.print(")\n") ;
      Serial.println("The button is long pressed.");   // 長按觸發
    } // end of else  if (buttonStatus == 1) 
  } //  end of if (int_flag) 
}

// 外部中斷服務函數（ISR - Interrupt Service Routine）
// 注意：此函式應盡量簡短，避免執行耗時操作
void ButtonInt() 
{
  // 當中斷發生時，僅設定旗標為 1，讓主迴圈在適當的時機處理按鈕事件
  int_flag = 1;  // 設定旗標為 1，通知主程式有中斷事件發生
}