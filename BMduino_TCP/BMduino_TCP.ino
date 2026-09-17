/**
 * BMduino_TCP.ino
 * 
 * 功能說明：
 * 1. 連線至指定的 WiFi 網路
 * 2. 連線至 TCP 伺服器
 * 3. 雙向資料傳輸：接收 TCP 數據並顯示，同時可將序列埠輸入的數據發送至 TCP 伺服器
 * 
 * 注意：此程式需要配合 TCP.h 函式庫使用
 */

#include "TCP.h"  // 匯入 TCP 通訊所需的函式庫

/**
 * 初始化設定函數
 * 在 Arduino 啟動時只執行一次
 */
void setup() 
{
  // 將 LED 腳位設定為低電位，確保 LED 初始狀態為關閉
  digitalWrite(LED, LOW); 
  
  // 初始化序列通訊，設定傳輸速率為 9600 bps
  // 用於與電腦的序列埠監控視窗進行溝通
  Serial.begin(9600); 
  
  // 初始化 WiFi 模組，準備進行無線網路連線
  Wifi.begin(); 
  
  // 輸出提示訊息，標示即將顯示 TCP 連線結果
  Serial.print("TCP Connect Result："); 

  // === WiFi 連線階段 ===
  // 嘗試連線至指定的 WiFi 存取點 (AP)
  // WIFI_SSID 和 WIFI_PASS 應在 TCP.h 中定義
  if(!Wifi.connectToAP(WIFI_SSID, WIFI_PASS)) 
  {
    // 若連線失敗，顯示失敗訊息
    Serial.print("WIFI fail,"); 
  }  
  else
  {
    // 若連線成功，顯示成功訊息
    Serial.print("WIFI success,"); 
  }

  // === TCP 伺服器連線階段 ===
  // 嘗試連線至指定的 TCP 伺服器
  // IP 和 IP_Port 應在 TCP.h 中定義
  if(!Wifi.connectTCP(IP, IP_Port))
  {
    // 若 TCP 連線失敗，顯示失敗訊息
    Serial.print("IP fail"); 
  } 
  else
  {
    // 若 TCP 連線成功，顯示成功訊息
    Serial.print("IP success"); 
  }
}

/**
 * 主循環函數
 * Arduino 會不斷重複執行此函數內的程式碼
 */
void loop() {
  
  // === TCP 數據接收處理 ===
  // 讀取從 TCP 伺服器傳來的數據
  tcpBuff = Wifi.readDataTcp();
  
  // 檢查是否收到數據（非 0 表示有數據）
  if(tcpBuff != 0) 
  {
    // 將收到的 TCP 數據顯示在序列埠監控視窗
    Serial.println(tcpBuff); 
  }

  // === 序列埠輸入處理 ===
  // 檢查序列埠是否有數據可讀
  // Serial.available() 會返回緩衝區中可讀取的字節數
  while (Serial.available() > 0)
  {
    // 從序列埠讀取一個字節，並存入 SerialBuff 緩衝區
    // resLen 為當前緩衝區長度，讀取後長度加 1
    SerialBuff[resLen++] = Serial.read(); 
    
    // 短暫延遲 10 毫秒，確保數據完全接收
    // 避免因讀取速度過快而漏掉數據
    delay(10); 
  }

  // === 數據發送處理 ===
  // 檢查緩衝區中是否有數據需要發送
  if(resLen > 0)
  {
    // 點亮 LED，表示開始傳輸數據
    digitalWrite(LED, HIGH); 
    
    // 嘗試將緩衝區中的數據透過 TCP 發送至伺服器
    // resLen: 要發送的數據長度
    // SerialBuff: 存放要發送數據的緩衝區
    if(Wifi.writeDataTcp(resLen, SerialBuff))    
    {
      // 若發送成功，顯示成功訊息
      Serial.println("Send data success"); 
      
      // 關閉 LED，表示傳輸完成
      digitalWrite(LED, LOW); 
    }
    
    // 呼叫自訂函數清空緩衝區，準備接收下一筆數據
    clearBuff(); 
  }
  
  // 注意：此處沒有 delay()，所以 loop 會非常快速重複執行
  // 這樣可以確保及時接收和發送數據
}

/**
 * 清空緩衝區的自訂函數
 * 用於重置接收緩衝區的狀態
 */
void clearBuff() {
  
  // 使用 memset 將 SerialBuff 陣列全部填入空字元 '\0'
  // RES_MAX_LENGTH 應在 TCP.h 中定義，表示緩衝區的最大長度
  memset(SerialBuff, '\0', RES_MAX_LENGTH); 
  
  // 將緩衝區長度計數器歸零
  // 表示緩衝區現在是空的
  resLen = 0; 
}

/**
 * 程式使用說明：
 * 
 * 1. 前置作業：
 *    - 確保 TCP.h 函式庫已正確安裝
 *    - 在 TCP.h 中定義以下參數：
 *      * WIFI_SSID：您的 WiFi 名稱
 *      * WIFI_PASS：您的 WiFi 密碼
 *      * IP：TCP 伺服器的 IP 位址
 *      * IP_Port：TCP 伺服器的埠號
 *      * RES_MAX_LENGTH：緩衝區最大長度
 * 
 * 2. 硬體接線：
 *    - LED 腳位需依照您的開發板定義
 *    - 確保 WiFi 模組已正確連接
 * 
 * 3. 操作方式：
 *    - 上傳程式後開啟序列埠監控視窗（鮑率 9600）
 *    - 觀察連線狀態訊息
 *    - 在序列埠輸入文字，將會發送至 TCP 伺服器
 *    - 從 TCP 伺服器收到的訊息會顯示在序列埠上
 * 
 * 4. LED 指示燈狀態：
 *    - 熄滅：正常待機狀態
 *    - 亮起：正在發送數據至 TCP 伺服器
 */