/********************* 程式整體運作解說 *************************/
/*
 * 【檔案名稱】BMduino_IRTemp_Collector_button.ino
 * 【程式用途】
 *   本程式為一套「紅外線溫度資料採集器」的 Arduino 主程式（Sketch），
 *   以「按鈕觸發」方式讀取紅外線溫度感測器（IR Temperature）量測值，
 *   將結果顯示於 OLED，並透過 Wi-Fi 以 HTTP GET 上傳至雲端伺服器。
 *
 *   主要整合以下五大功能模組：
 *     1. commlib.h      ：通訊相關共用函式（封包處理、緩衝區管理等）。
 *     2. LedButtonLib.h ：LED 按鈕模組控制（按鈕讀取、LED 指示）。
 *     3. OledLib.h      ：OLED 128x64 顯示模組控制（清屏、畫圖、印字串）。
 *     4. TCP.h          ：Wi-Fi／TCP 連線功能封裝（連線、MAC 取得等）。
 *     5. IRTempLib.h    ：紅外線溫度感測器控制（初始化、讀取溫度）。
 *     6. clouding.h     ：雲端上傳封裝（RESTful API，含 SendtoClouding()）。
 *
 * 【與 Oxygen_DataCollectorV2 的主要差異】
 *   - 觸發方式：本程式為「按鈕觸發」（被動），非自動連續/查詢模式。
 *   - 感測器  ：紅外線溫度模組（IRTempLib）而非血氧／心率模組。
 *   - 上傳時機：按鈕按下且成功讀取溫度後才上傳。
 *   - 迴圈節奏：每次成功讀取後 delay(6000)，即每 6 秒一個循環。
 *
 * 【整體運作流程】
 *   ┌─────────────┐
 *   │   setup()   │
 *   └──────┬──────┘
 *          │
 *          ├─(1) initAll()：啟動序列埠（9600 bps）並初始化感測模組。
 *          │       內部呼叫 initSensor()：
 *          │         initOled() → delay(2000)
 *          │         → initIRTemperature() → intLedButton()
 *          │
 *          ├─(2) delay(200)：硬體穩定延遲。
 *          │
 *          ├─(3) INITtWIFI()：初始化 Wi-Fi 連線，
 *          │       並呼叫 ShowWiFiInformation() 取回 MAC/SSID/IP。
 *          │
 *          ├─(4) 開機歡迎畫面：於 OLED 顯示廠牌 LOGO（128x64 全螢幕），
 *          │       停留 3 秒後清除畫面。
 *          │
 *          ├─(5) 顯示網路資訊：將 MAC（第 0 列）與 IP（第 2 列）
 *          │       顯示於 OLED，方便辨識裝置與除錯。
 *          │
 *          └─(6) 印出 "Enter Loop()"，進入主迴圈。
 *
 *   ┌─────────────┐
 *   │   loop()    │  ← 主迴圈持續執行（按鈕觸發式）
 *   └──────┬──────┘
 *          │
 *          ├─(1) 判斷 getButton(1) != 0（第一顆按鈕是否按下）：
 *          │       ├─ 是（按下）：IRactive = true，繼續往下執行。
 *          │       └─ 否（未按下）：IRactive = false，return 跳回開頭。
 *          │
 *          ├─(2) 若 IRactive 為真：
 *          │       ├─ bodytemp = readIRTemperature()：讀取紅外線溫度。
 *          │       ├─ Serial 印出溫度值。
 *          │       ├─ showMsgonOled(...)：於 OLED 第 4 列顯示溫度。
 *          │       ├─ SendtoClouding()：上傳資料至雲端。
 *          │       └─ IRactive = false：重設旗標。
 *          │
 *          └─(3) delay(6000)：延遲 6 秒，避免過度頻繁觸發。
 *
 * 【重要全域變數】
 *   - SSIDData ：目前連線的 Wi-Fi 熱點名稱（SSID）。
 *   - IPData   ：由 AP 分配給本機的 IPv4 位址。
 *   - MacData  ：本機 MAC 位址（裝置唯一 ID，上雲或資料庫主鍵）。
 *   - IRactive ：是否啟動溫度讀取（true=讀取、false=不讀取）。
 *   - bodytemp ：紅外線量測到的物體溫度（由 IRTempLib 提供）。
 *
 * 【硬體周邊】
 *   - OLED 128x64 顯示器（由 OledLib 封裝）。
 *   - 紅外線溫度感測器（由 IRTempLib 封裝）。
 *   - LED 按鈕模組（由 LedButtonLib 封裝，按鈕編號 1）。
 *   - Wi-Fi 模組（由 TCP.h 封裝）。
 *
 * 【注意事項】
 *   - 序列埠鮑率固定為 9600 bps，PC 端監控視窗須設定一致。
 *   - loop() 為「按鈕觸發式」：未按下按鈕時直接 return，
 *     不會執行讀取與上傳，可有效降低耗電與網路流量。
 *   - 按下按鈕後會進入 delay(6000)，此期間無法再次觸發，
 *     可避免連續誤觸造成重複上傳。
 *   - bodytemp 變數由 IRTempLib.h 提供，本檔未另行宣告。
 *   - SendtoClouding() 由 clouding.h 提供，需確保 Wi-Fi 已連線。
 */
// ------- 全域變數宣告區 -----------
String SSIDData;   // 儲存 WiFi 熱點名稱 (SSID)
String IPData;     // 儲存 WiFi 分配到的 IP 位址
String MacData;     // 儲存 WiFi 分配到的 MAC Address
// MacData 在其他函式中宣告，作為全域變數
boolean IRactive = false ;  //啟動量溫度
// ------- 感測模組函式與外部函式引用宣告區 -----------
#include <String.h>    // Arduino 內建字串處理函式庫
#include "commlib.h"   // 通訊相關的共用函式庫（可能包含封包處理、緩衝區管理等）

#include "LedButtonLib.h"       // LED Button模組自訂函式

#include "OledLib.h"   // 自訂 OLED 顯示模組函式庫（提供 OLED 初始化、文字繪製、清屏等功能）
#include "TCP.h"       // TCP 通訊函式庫（包含 WiFi 初始化、MAC 取得等函式）
#include "IRTempLib.h"       // IR Temperature紅外線溫度模組自訂函式


#include "clouding.h"       // REST Ful API 基本模組

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統
void INITtWIFI();                  // 初始化 WiFi 網路連線
void showTitleonOled(String ss,int row);   // 顯示標題文字於 OLED 第一列
void showIPonOled(String ss,int row);      // 顯示 IP 位址於 OLED
void showDeviceonOled(String ss,int row);  // 顯示裝置 Device ID 於 OLED
void ShowWiFiInformation();  //顯示wifi的基本參數



void setup() 
{
    initAll();       // 初始化整體系統（啟動序列埠、初始化 OLED、Relay）
  delay(200);      // 延遲 200ms，確保硬體模組穩定
  INITtWIFI();     // 初始化 WiFi 網路，並取得 SSID、IP 與 MAC 資料

  //---------------------------------
  clearScreen();  // 清除 OLED 螢幕
  // 顯示 BEST MODULES 的 LOGO
  drawPicture(0, 0, BestModule_LOGO, 128, 64);
  delay(3000);    // LOGO 顯示 3 秒
  clearScreen();  // 再次清除螢幕

  // 顯示系統資訊於 OLED
  showTitleonOled(MacData,0);  // 在 OLED 顯示 MAC 位址
  showIPonOled(IPData,2);      // 在 OLED 顯示 IP 位址
  //----------------------------
  Serial.println("Enter Loop()"); // 提示已經進入主迴圈 loop()
}



void loop()
{
  if (getButton(1) != 0 ) //第一顆按鈕有按下
  {
      IRactive = true ;//設定讀取溫度
  }
  else
  {
      IRactive = false ;//設定不讀取溫度
      return ;
  }

  if (IRactive)
  {
  bodytemp = readIRTemperature(); //讀取紅外線模組取得量測溫度
  Serial.print("讀取到溫度：(");
  Serial.print(bodytemp);
  Serial.print(") °C \n");
//------------處理顯示OLED問題--------------
  showMsgonOled("Temp:("+String(bodytemp)+").C",4); //列印Message於OLED上
  //------------雲端資料傳送問題--------------
  SendtoClouding() ;    //傳送感測資料到雲端
  IRactive = false ;//設定不讀取溫度
  }
  delay(6000) ; //延遲六秒鐘
}

// ------------------ 系統初始化區 ------------------

// 初始化所有感測模組
void initSensor()
{
  initOled();    // 初始化 OLED 12864 (0.96吋 OLED BMD31M090)
  delay(2000);   // 延遲 2 秒，等待顯示模組穩定
  initIRTemperature() ;   //初始化紅外線模組
  intLedButton();   //初始化LEDButton 模組
}

// 初始化整體系統
void initAll()
{
  Serial.begin(9600);  // 啟動序列埠，速率 9600 bps
  initSensor();        // 呼叫初始化感測模組
}

// 初始化 WiFi
void INITtWIFI()
{
  initWiFi();   // 初始化 WiFi 功能
  ShowWiFiInformation();  //顯示wifi的基本參數
}

void ShowWiFiInformation()  //顯示wifi的基本參數
{
  //顯示wifi的基本參數
  MacData = GetMAC();     // 取得裝置的 MAC 位址
  Serial.println("---MAC Address----");
  Serial.println(MacData);

  Serial.println("");
  Serial.println("---wifi access point----");

  SSIDData = GetSSID();   // 取得連線的 WiFi 熱點名稱
  Serial.println(SSIDData);

  Serial.println("---Show IP Address----");
  IPData = GetIP();       // 取得裝置分配到的 IP 位址
  Serial.println(IPData);


}