/********************* 程式整體運作解說 *************************/
/*
 * 【檔案名稱】clouding.h（紅外線溫度版）
 * 【程式用途】
 *   本檔案負責將紅外線溫度感測器量測到的體溫資料，透過 Wi-Fi 以
 *   HTTP GET 方式上傳到遠端雲端伺服器（RESTful API），並檢查
 *   伺服器回傳內容以判斷上傳是否成功。
 *
 *   本檔為「紅外線溫度採集器」（BMduino_IRTemp_Collector_button.ino）
 *   專用的雲端上傳版本，與血氧版 clouding.h 的差異在於：
 *     - API 路徑：/c08/bodytemp/dataadd.php（體溫）
 *     - 上傳參數：MAC + bodytemp（單一溫度值）
 *     - 血氧版為  ：/c08/bloodoxygen/dataadd.php（MAC + bd + hb）
 *
 * 【雲端伺服器資訊】
 *   - 伺服器位址：http://iot.arduino.org.tw
 *   - 服務埠號  ：8888
 *   - API 路徑  ：/c08/bodytemp/dataadd.php
 *   - 請求方法  ：HTTP GET
 *
 * 【上傳資料格式（URL 參數）】
 *   ?MAC=%s&bodytemp=%s
 *     - MAC      ：裝置唯一識別碼（字串，如 11BBCCDDEEFF）
 *     - bodytemp ：量測到的體溫（字串，如 41.3）
 *
 *   完整範例：
 *   http://iot.arduino.org.tw:8888/c08/bodytemp/dataadd.php
 *       ?MAC=11BBCCDDEEFF&bodytemp=41.3
 *
 * 【HTTP 請求拆解】
 *   ┌────────────┬──────────────────────────────────────────┐
 *   │  Host      │ iot.arduino.org.tw:8888                  │
 *   │  Path      │ /c08/bodytemp/dataadd.php                │
 *   │  Parameters│ ?MAC=11BBCCDDEEFF&bodytemp=41.3          │
 *   └────────────┴──────────────────────────────────────────┘
 *
 * 【整體運作流程】
 *   ┌──────────────────────┐
 *   │  SendtoClouding()    │  ← 由主程式 loop() 在按鈕觸發讀到溫度後呼叫
 *   └──────────┬───────────┘
 *              │
 *              ├─(1) sprintf()：將 MAC 與 bodytemp 格式化為 URL 參數字串，
 *              │       存入 dbagentstr[300] 暫存區。
 *              │       ※ 使用 String(bodytemp).c_str() 將 float 轉為字串。
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
 *   - webresponse ：伺服器回傳的 HTTP GET 結果字串。
 *   ※ MacData 與 bodytemp 由其他檔案提供（TCP.h 與 IRTempLib.h）。
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
 *   - 依賴 IRTempLib.h 提供的全域變數 bodytemp。
 *   - 由主程式 BMduino_IRTemp_Collector_button.ino 在按鈕觸發並
 *     成功讀取溫度後呼叫 SendtoClouding()。
 *
 * 【注意事項】
 *   - 溫度值格式：使用 String(bodytemp).c_str() 將 float 轉為字串，
 *     預設小數位數依 Arduino String 建構子（通常為 2 位）。
 *     若需固定小數位（如 1 位），可改用 String(bodytemp, 1)。
 *   - sprintf 的 %s 需傳入 C 字串（使用 String::c_str()）。
 *   - 回應判斷以字串 "Successful" 為準，需與伺服器端輸出一致。
 *   - Wi-Fi 未連線時會嘗試重新初始化，可能造成短暫延遲。
 *   - 註解中提到的 Tvalue／Hvalue 為溫濕度版範例殘留文字，
 *     本檔實際上傳的是 MAC 與 bodytemp 兩個參數。
 */
//http://iot.arduino.org.tw:8888/c08/bodytemp/dataadd.php?MAC=11BBCCDDEEFF&bodytemp=41.3

#define  HTTPGET_PORT_HTTP 80 
#define  HTTPGET_PORT_HTTPS 443


#define ServerPort 8888
String ServerURL = "http://iot.arduino.org.tw";
#define dbagent "/c08/bodytemp/dataadd.php?MAC=%s&bodytemp=%s"
char dbagentstr[300] ; //sprint 使用之暫存區
 String connectstr ;    //一個空的字串變數，後續用來動態組成完整的 RESTful 請求參數。

String webresponse ;//取得http get回傳值

//http://iot.arduino.org.tw:8888/bigdata/bodytemp/dataadd.php?MAC=11BBCCDDEEFF&bodytemp=41.3
// host is  ==>iot.arduino.org.tw:8888
//  app program is ==> /bigdata/bodytemp/dataadd.php
//  App parameters ==> ?MAC=11BBCCDDEEFF&bodytemp=41.3
 
 /*
   完整的 HTTP 請求範例，分解如下：
  主機位址：iot.arduino.org.tw:8888（域名與通訊埠）。
  應用程式路徑：/bigdata/bodytemp/dataadd.php（伺服器上的 PHP 程式，用來接收並處理資料）。
  參數：?MAC=11BBCCDDEEFF&bodytemp=41.3，表示傳送的資料：
  MAC：設備的唯一識別碼（例如 112233445566）。
  bodytemp：溫度值（例如 65.1）。
 
 
 */

void SendtoClouding() ;    //傳送感測資料到雲端

void SendtoClouding()     //傳送感測資料到雲端
{

sprintf(dbagentstr,dbagent,MacData.c_str(),String(bodytemp).c_str()) ;
   connectstr = String(dbagentstr) ;
   /*
   組成GET Format 的Resetful  的 Parameters 字串
   connectstr：動態組成 RESTful 請求的參數部分：
   MacData：設備的 MAC 位址（假設已在程式其他地方定義）。
   Tvalue：溫度值，轉換成字串格式。
   Hvalue：濕度值，轉換成字串格式。
   */
          
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