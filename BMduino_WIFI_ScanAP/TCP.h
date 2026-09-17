#ifndef _BMC81M001_H__  // 防止重複定義 Header（Header Guard）
#define _BMC81M001_H__

//*********************************************//
//****************** 引用函式庫 ****************//
//*********************************************//
#include "BMC81M001.h"   // 引入 BMC81M001 WiFi 模組控制函式庫
#include <String.h>      // 引入 Arduino 內建 String 類別（提供字串處理功能）

//*********************************************//
//*********** WiFi 熱點連線設定區 ***************//
//*********************************************//

/*** 設定要連線的 WiFi 熱點帳號與密碼 ***/
#define WIFI_SSID "NUKIOT"     // 無線網路的 SSID (網路名稱)
#define WIFI_PASS "iot12345"   // 無線網路的密碼 (Password)


//*********************************************//
//************* IO 腳位與變數宣告區 *************//
//*********************************************//

int LED = 13;   // 指定板上 LED 的腳位，常用於顯示狀態（如連線中、傳輸中）

#define DEB_CNT        50      // 除錯延遲常數 (50 毫秒)
#define RES_MAX_LENGTH 200     // 串列接收緩衝區的最大長度 (200 Bytes)

char  SerialBuff[RES_MAX_LENGTH];  // 串列接收資料的暫存緩衝區
char  data[30];                    // 關鍵資料暫存用的字元陣列
int   resLen;                      // 實際接收到的資料長度
int   nKeyBuf;                     // 用於記錄 Key 資料或緩衝索引
String DATA_BUF;                   // 用來暫存資料的 String 變數
String tcpBuff;                    // TCP 傳輸使用的暫存字串

#endif  // Header Guard 結束


//*********************************************//
//************* WiFi 模組物件建立區 *************//
//*********************************************//

// BMC81M001 Wifi(6,7);        // 若使用軟體序列埠 (Software Serial)，則 RX=6, TX=7
// BMC81M001 Wifi(&Serial1);   // 若使用硬體序列埠 Serial1，請取消註解此行
BMC81M001 Wifi(&Serial2);      // 使用 BMduino 板上的硬體 Serial2（RX2/TX2）連接 WiFi 模組


//*********************************************//
//************ 自訂函式前置宣告區 ***************//
//*********************************************//

void initWiFi();       // 初始化 WiFi 模組
String GetMAC();       // 取得 WiFi 模組的 MAC 位址
String GetSSID();      // 取得目前連線的 WiFi 熱點名稱
String GetIP();        // 取得分配的 IP 位址
String GetGateWay();   // 取得連線熱點的閘道器 IP 位址
String GetsubMask();   // 取得連線熱點的子網路遮罩
String ScanAP();       // 掃描附近可用的 WiFi 熱點


//*********************************************//
//*************** 初始化 WiFi 函式 ***************//
//*********************************************//

void initWiFi()
{
   Wifi.begin();      // 啟動 WiFi 模組，初始化內部通訊功能
   Wifi.reset();      // 重設模組（清除暫存狀態與連線資訊）
   delay(1000);       // 延遲 1 秒，確保模組穩定

   Serial.print("init WIFI："); 

   // 嘗試連線至指定的 WiFi 熱點
   if (!Wifi.connectToAP(WIFI_SSID, WIFI_PASS)) 
   {
      Serial.print("WIFI fail,");     // 若回傳 false，表示 WiFi 連線失敗
   }  
   else
   {
      Serial.print("WIFI success,");  // 若成功連線至 WiFi 熱點
   }

   delay(500);  // 等待網路連線完全建立
}


//*********************************************//
//*************** 取得 MAC 位址函式 ***************//
//*********************************************//
String GetMAC()
{
   delay(500);                // 等待模組回應穩定
   String tmp = "";           // 宣告暫存字串
   tmp = Wifi.getMacAddress(); // 呼叫函式取得模組的 MAC 位址
   tmp.toUpperCase();          // 將字母轉成大寫（格式更整齊）
   return tmp;                 // 回傳 MAC 位址字串
}


//*********************************************//
//*************** 取得 SSID 名稱函式 ***************//
//*********************************************//
String GetSSID()
{
   delay(500);          // 等待模組穩定
   String tmp = "";     // 暫存字串
   if (Wifi.getStatus())  // 確認模組目前處於連線狀態
   {
      tmp = Wifi.getSSID();  // 從模組取得目前連線的 SSID 名稱
      tmp.toUpperCase();     // 統一轉成大寫
   }
   else
   {
      tmp = "";   // 若未連線，回傳空字串
   }
   return tmp;
}


//*********************************************//
//*************** 取得 IP 位址函式 ***************//
//*********************************************//
String GetIP()
{
   delay(500);          // 等待模組穩定
   String tmp = "";     // 暫存字串
   if (Wifi.getStatus())  // 若模組已成功連線 WiFi
   {
      tmp = Wifi.getIP();   // 取得 DHCP 分配的 IP 位址
      tmp.toUpperCase();
   }
   else
   {
      tmp = "";
   }
   return tmp;          // 回傳 IP 位址字串
}


//*********************************************//
//*************** 取得 Gateway 位址函式 ***************//
//*********************************************//
String GetGateWay()
{
   delay(500);
   String tmp = "";
   if (Wifi.getStatus())
   {
      tmp = Wifi.getGateway();  // 取得閘道器 (Router Gateway) 位址
      tmp.toUpperCase();
   }
   else
   {
      tmp = "";
   }
   return tmp;  // 回傳 Gateway IP 位址
}


//*********************************************//
//*************** 取得 Subnet Mask 函式 ***************//
//*********************************************//
String GetsubMask()
{
   delay(500);
   String tmp = "";
   if (Wifi.getStatus())
   {
      tmp = Wifi.getMask();  // 取得子網路遮罩 (Subnet Mask)
      tmp.toUpperCase();
   }
   else
   {
      tmp = "";
   }
   return tmp;  // 回傳 Subnet Mask 位址
}


//*********************************************//
//*************** 掃描附近 WiFi 熱點函式 ***************//
//*********************************************//
String ScanAP()
{
   delay(500);
   String tmp = "";
   if (Wifi.getStatus())
   {
      tmp = Wifi.SSID();  // 呼叫模組內部函式列出可用 WiFi 清單
      tmp.toUpperCase();  // 統一格式
   }
   else
   {
      tmp = "";
   }
   return tmp;  // 回傳可用 WiFi 熱點清單
}

/*
區塊	功能	說明
Header Guard	防止重複包含	確保此標頭檔在多個檔案中僅被編譯一次
WIFI_SSID / WIFI_PASS	WiFi 連線設定	儲存要連線的無線網路名稱與密碼
BMC81M001 Wifi(&Serial2)	建立 WiFi 控制物件	使用硬體序列埠 Serial2 通訊（速度快且穩定）
initWiFi()	初始化模組	開啟、重設並嘗試連線至指定 WiFi 熱點
GetMAC() / GetSSID() / GetIP() / GetGateWay() / GetsubMask()	網路資訊擷取	讀取模組內部記錄的各項網路連線資訊
ScanAP()	WiFi 熱點掃描	搜尋目前可連線的所有 WiFi 熱點
*/