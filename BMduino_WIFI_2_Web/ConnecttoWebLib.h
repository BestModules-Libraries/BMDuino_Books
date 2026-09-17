// 範例網址格式 (實際上就是 Arduino 要送出的 HTTP GET 請求)
// 說明：此為一般雲端平台的 API 端點格式，ESP32 / Wi-Fi 模組會組成
//      http://網址:埠號/對應的 RESTful 參數
//      例如：http://iot.arduino.org.tw:8888/xxx?MAC=xxxx&value=123
// ---------------------------------------------------------------
// =========================【通訊埠定義區】=========================

// 定義 HTTP/HTTPS 的預設通訊埠
#define  HTTPGET_PORT_HTTP 80       // HTTP 預設連接埠 (明碼傳輸)
#define  HTTPGET_PORT_HTTPS 443     // HTTPS 預設連接埠 (加密傳輸)

// 定義伺服器的專用通訊埠
// 說明：當雲端平台有自行修改 API 的服務埠號時，需手動指定。
//       若平台使用 80，代表一般 HTTP 正常瀏覽即可。
#define ServerPort 8888                // 伺服器對外提供服務的埠號（HTTP 80）

// 雲端伺服器網址字串
// 說明：此為 WiFi.http_begin() 會使用到的「主機位址」
//       後面只需把 GET 參數字串加在 URL path 即可。
//String ServerURL = "https://www.drmaster.com.tw/";   // 伺服器的主機域名
String ServerURL = "http://iot.arduino.org.tw";   // 伺服器的主機域名
String dbagent =  "/bigdata/dhtdata/dataadd.php?MAC=112233445566&T=65.1&H=76" ;
// 宣告字串變數，用來接收雲端伺服器回傳的結果
String webresponse;     // 儲存 HTTP GET 的回應內容（例如：成功、錯誤、JSON 資料等）
//http://iot.arduino.org.tw:8888/bigdata/dhtdata/dataadd.php?MAC=112233445566&T=65.1&H=76


// =========================【函式宣告區】=========================

// Connect2Web()：實作連接到雲端網站並執行 HTTP GET
// webutl：代表 網站網址之參數字串，例如："http://iot.arduino.org.tw:8888/bigdata/dhtdata/dataadd.php?MAC=112233445566&T=65.1&H=76"
void Connect2Web(String webutl);    // 實作：連接到雲端網站



// =========================【函式實作區】=========================

// 實作：連接到雲端網站
void Connect2Web(String webutl) // 實作：連接到雲端網站
{
    // 除錯用途：將組好的 URL 字串印到序列監控器
    // 目的：方便檢查組出的 HTTP GET 字串是否正確
    Serial.println(webutl);  // 將組合好的 RESTful API 字串輸出去


    // =====================【檢查 WiFi 是否已連線】=====================

    // Wifi.getStatus() 代表目前模組是否已成功連線到 Wi-Fi AP
    if (Wifi.getStatus())
    {
        // ------------------------ 1. 開始 HTTP GET 連線 ------------------------
        // 參數：
        // ServerURL → 主機網址
        // ServerPort → 埠號，例如 80
        // webutl → 後方的 API 路徑與參數
        Wifi.http_begin(ServerURL, ServerPort, webutl);   // 開始建立 HTTP 連線

        // ------------------------ 2. 執行 GET 操作 -------------------------
        Wifi.http_get();  // 實際送出 GET 請求

        // ------------------------ 3. 取得回傳字串 ------------------------
        webresponse = Wifi.http_getString();  // 取得伺服器回應內容（例如 JSON）

        // ------------------------ 4. 顯示回應內容 ------------------------
        Serial.println(webresponse);   // 印出 GET 結果
    }
    else
    {
        // =====================【WiFi 未連線，執行初始化】=====================

        // 若 WiFi 尚未連線，則呼叫 initWiFi() 重新建立連線
        // initWiFi() 通常定義於 TCP.h 或 BMC81M001.h
        // 會處理：搜尋 SSID → 嘗試連線 → 等待 IP → 取得 MAC 等資訊
        initWiFi();

        // 印出美觀用途的分隔線
        Serial.println("---MAC Address----");

        // 印出目前 WiFi 所取得的 MAC Address（或由模組回傳的網路資訊）
        // MacData 為全域變數，代表 WiFi 模組的 MAC 位址字串
        Serial.println(MacData);


        // 重新執行與雲端伺服器的 HTTP GET 流程
        Wifi.http_begin(ServerURL, ServerPort, webutl);  // 開始 HTTP GET 連線
        Wifi.http_get();                                 // 實際 GET
        webresponse = Wifi.http_getString();             // 取得 GET 回應
        Serial.println(webresponse);                     // 印出結果
    }
}
