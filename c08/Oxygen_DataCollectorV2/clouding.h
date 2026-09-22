/********************* 程式整體運作解說 *************************/
/*
 * 【檔案名稱】clouding.h
 * 【程式用途】
 *   本檔案負責將血氧／心率感測器量測到的資料，透過 Wi-Fi 以
 *   HTTP GET 方式上傳到遠端雲端伺服器（RESTful API），並檢查
 *   伺服器回傳內容以判斷上傳是否成功。
 *
 * 【雲端伺服器資訊】
 *   - 伺服器位址：http://iot.arduino.org.tw
 *   - 服務埠號  ：8888
 *   - API 路徑  ：/c08/bloodoxygen/dataadd.php
 *   - 請求方法  ：HTTP GET
 *
 * 【上傳資料格式（URL 參數）】
 *   ?MAC=%s&bd=%s&hb=%d
 *     - MAC：裝置唯一識別碼（字串，如 112233445566）
 *     - bd ：血氧值（浮點數字串，如 97.45）
 *     - hb ：心跳值（整數，如 83）
 *
 *   完整範例：
 *   http://iot.arduino.org.tw:8888/c08/bloodoxygen/dataadd.php
 *       ?MAC=112233445566&bd=97.45&hb=83
 *
 * 【HTTP 請求拆解】
 *   ┌────────────┬──────────────────────────────────────────┐
 *   │  Host      │ iot.arduino.org.tw:8888                  │
 *   │  Path      │ /c08/bloodoxygen/dataadd.php             │
 *   │  Parameters│ ?MAC=AABBCCDDEEFF&bd=97.45&hb=83         │
 *   └────────────┴──────────────────────────────────────────┘
 *
 * 【整體運作流程】
 *   ┌──────────────────────┐
 *   │  SendtoClouding()    │  ← 由主程式 loop() 在 readok=1 時呼叫
 *   └──────────┬───────────┘
 *              │
 *              ├─(1) sprintf()：將 MAC、血氧、心跳格式化為 URL 參數字串
 *              │       存入 dbagentstr[300] 暫存區。
 *              │       ※ 血氧值先除以 100 再轉字串（依實際資料格式調整）。
 *              │
 *              ├─(2) 將 C 字元陣列轉為 String（connectstr），
 *              │       便於後續 WiFi 函式使用。
 *              │
 *              ├─(3) 印出組合好的 URL 字串到序列埠（除錯用）。
 *              │
 *              └─(4) 判斷 Wi-Fi 連線狀態 Wifi.getStatus()：
 *                     ├─ 已連線：直接進行 HTTP GET 上傳。
 *                     └─ 未連線：先呼叫 initWiFi() 重新連線，
 *                                並重新取得 MAC（GetMAC()），
 *                                再進行 HTTP GET 上傳。
 *
 *   HTTP GET 上傳步驟（兩種路徑相同）：
 *     ┌─────────────────────────────────────────────┐
 *     │ Wifi.http_begin(ServerURL, ServerPort,      │
 *     │                 connectstr)  → 建立請求     │
 *     │ Wifi.http_get()              → 送出 GET     │
 *     │ Wifi.http_getString()        → 取回回應字串 │
 *     │ 檢查回應是否含 "Successful"  → 印出成功/失敗│
 *     │ Wifi.http_end()              → 結束連線     │
 *     └─────────────────────────────────────────────┘
 *
 * 【重要全域變數】
 *   - ServerURL   ：雲端伺服器位址（http://iot.arduino.org.tw）。
 *   - ServerPort  ：伺服器服務埠號（8888）。
 *   - dbagentstr  ：C 字元陣列（300 bytes），存放 sprintf 格式化結果。
 *   - connectstr  ：String 物件，存放完整 RESTful 請求字串。
 *   - MacData     ：本機網卡 MAC 位址（裝置唯一識別碼）。
 *   - webresponse ：伺服器回傳的 HTTP GET 結果字串。
 *
 * 【巨集定義】
 *   - HTTPGET_PORT_HTTP  ：80（HTTP 預設埠）
 *   - HTTPGET_PORT_HTTPS ：443（HTTPS 預設埠）
 *   - ServerPort         ：8888（本專案伺服器實際服務埠）
 *   - dbagent            ：API 路徑與參數格式字串。
 *
 * 【與其他模組的關係】
 *   - 依賴 TCP.h / BMC81M001.h 提供的 Wifi 物件與相關函式：
 *       Wifi.getStatus()、Wifi.http_begin()、Wifi.http_get()、
 *       Wifi.http_getString()、Wifi.http_end()。
 *   - 依賴 initWiFi()、GetMAC()（定義於 TCP.h）進行重新連線與
 *     取得 MAC。
 *   - 由主程式 Oxygen_DataCollectorV2.ino 在 readok != 0 時呼叫
 *     SendtoClouding()。
 *
 * 【注意事項】
 *   - 血氧值格式：程式中將 oxyvalue 除以 100 後再轉字串，
 *     請依實際感測器輸出格式調整（例如直接整數或 /10）。
 *   - sprintf 的 %s 需傳入 C 字串（使用 String::c_str()）。
 *   - 回應判斷以字串 "Successful" 為準，需與伺服器端輸出一致。
 *   - Wi-Fi 未連線時會嘗試重新初始化，可能造成短暫延遲。
 */
// 範例網址格式 (實際上就是 Arduino 要送出的 HTTP GET 請求)
// http://iot.arduino.org.tw:8888/c08/bloodoxygen/dataadd.php?MAC=112233445566&bd=97.45&hb=83

// 定義 HTTP/HTTPS 的預設通訊埠
#define  HTTPGET_PORT_HTTP 80     // HTTP 預設連接埠
#define  HTTPGET_PORT_HTTPS 443   // HTTPS 預設連接埠

// 定義伺服器的專用通訊埠
#define ServerPort 8888           // 伺服器的實際服務埠號
String ServerURL = "http://iot.arduino.org.tw";   // 伺服器位址

// 設定 RESTful API 的資料上傳格式（使用 %s、%f、%d 作為格式化字元）
// MAC = 裝置的唯一識別碼
// bd = 血氧值 (float，小數點一位)
// hb = 心跳值 (整數)
#define dbagent "/c08/bloodoxygen/dataadd.php?MAC=%s&bd=%s&hb=%d"
//http://iot.arduino.org.tw/c08/bloodoxygen/dataadd.php?MAC=AABBCCDDEEFF&bd=%6.3f&hb=%d
// 宣告暫存變數，用來存放 sprintf 格式化後的 URL 字串
char dbagentstr[300];   // C 字元陣列，作為 sprintf() 格式化字串的暫存區
String connectstr;      // 字串物件，用來儲存完整的 RESTful 請求字串
String MacData;         // 紀錄本機網卡 MAC 位址
String webresponse;     // 儲存伺服器回傳的 HTTP GET 結果

/*
  HTTP 完整請求拆解：
  Host：iot.arduino.org.tw:8888
  Path：/c08/bloodoxygen/dataadd.php
  Parameters：?MAC=AABBCCDDEEFF&bd=97.45&hb=83
    - MAC：裝置的唯一識別碼
    - bd：血氧值
    - hb：心跳值
*/

// 函式宣告：傳送感測資料到雲端
void SendtoClouding();

// 實作：傳送感測資料到雲端伺服器
void SendtoClouding()
{
  // 使用 sprintf() 將感測數據格式化成完整的 URL 參數字串
  // MacData：裝置的 MAC 位址
  // oxyvalue：血氧值 (轉成浮點數並縮放，這裡假設 oxyvalue 已經是百分比 *100)
  // hbvalue：心跳值
  sprintf(dbagentstr, dbagent, MacData.c_str(), String((float)((float)oxyvalue/100)), hbvalue);

  // 將 C 字元陣列轉為 String，方便後續使用 WiFi 函式
  connectstr = String(dbagentstr);

  // 除錯用途：將組好的 URL 字串印到序列監控器
  Serial.println(connectstr) ;//將組合好的參數字串輸出到序列監控視窗，用於除錯
 
 if  (Wifi.getStatus())
 {
    Wifi.http_begin(ServerURL,ServerPort,connectstr);//begin http get 
    Wifi.http_get();//http get opration
   webresponse = Wifi.http_getString();//get http result
    Serial.println(webresponse);//get http result
   if (webresponse.indexOf("Successful") != -1) //判斷回傳內容是否有"Successful" 字串
   {Serial.println("Http GET Successful"); /*印出http get上傳成功*/} 
   else
    {Serial.println("Http GET Fail"); /*印出http get上傳失敗*/ }
    Wifi.http_end(); //end of http get
   }
 else
 {
    initWiFi();              // 執行 WiFi 模組初始化與連線（定義於 TCP.h / BMC81M001.h）
      MacData = GetMAC() ; //取得 MAC 位址字串
    Serial.println("---MAC Address----"); // 分隔線，美觀用途
    Serial.println(MacData); // 印出取得 連接上的SSID熱點之後閘道器IP位址
   Wifi.http_begin(ServerURL,ServerPort,connectstr);//begin http get 
    Wifi.http_get();//http get opration
   webresponse = Wifi.http_getString();//get http result
    Serial.println(webresponse);//get http result
   if (webresponse.indexOf("Successful") != -1) //判斷回傳內容是否有"Successful" 字串
   {Serial.println("Http GET Successful"); /*印出http get上傳成功*/} 
   else
    {Serial.println("Http GET Fail"); /*印出http get上傳失敗*/ }
    Wifi.http_end(); //end of http get
 }
}