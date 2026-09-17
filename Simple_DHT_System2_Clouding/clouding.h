//---------全域define網路通訊埠變數，定義通訊埠-------
#define  HTTPGET_PORT_HTTP 80
#define  HTTPGET_PORT_HTTPS 443

//--------HTTP GET URL字串變數定義區---------
#define ServerPort 8888
String ServerURL = "http://iot.arduino.org.tw";
#define dbagent "/bmduino/dhtdata/dataadd.php?MAC=%s&T=%4.1f&H=%4.1f"
char dbagentstr[300] ; //sprint 使用之暫存區
String connectstr ;    //一個空的字串變數，後續用來動態組成完整的 RESTful 請求參數。
String webresponse ;//取得http get回傳值

//----------自定義函式區宣告--------------
void SendtoClouding(); //傳送感測資料到感測資料雲端代理人之函式

//------傳送感測資料到感測資料雲端代理人之函式-------
void SendtoClouding() //傳送感測資料到感測資料雲端代理人之函式
{

// 自訂函數，用來將感測器的資料（例如溫度和濕度）透過 HTTP GET 請求傳送到雲端伺服器。
//http://iot.arduino.org.tw:8888/bmduino/dhtdata/dataadd.php?MAC=AABBCCDDEEFF&T=34&H=34
// 雲端主機  ==>iot.arduino.org.tw:8888
//  感測資料雲端代理人 program is ==> bigdata/dhtdata/dhDatatadd.php
//  感測資料雲端代理人 parameters ==> ?MAC=AABBCCDDEEFF&T=34&H=34
//---------透過格式化字串，組合HTTP GET URL字串--------
sprintf(dbagentstr,dbagent,MacData.c_str(),TValue,HValue) ;
connectstr = String(dbagentstr) ;

 //---------印出HTTP GET URL 變數內容-------         
 Serial.println(connectstr) ;//將組合好的參數字串輸出到序列監控視窗，用於除錯
 
 if  (Wifi.getStatus())	//-----網路狀態是否連線成功
 {
    //-----網路狀態連線成功則進行
    //-----透過網路，使用http get方式連線------
	  Wifi.http_begin(ServerURL,ServerPort,connectstr);//透過網路，使用http get方式連線 
	//-----讀取http get連線回傳頁面資料------
	Wifi.http_get();//運行http get方式連線，並取得回傳頁面
   webresponse = Wifi.http_getString();//取得回傳頁面內容字串回傳到webresponse
    Serial.println(webresponse);//印出回傳頁面內容字串
   if (webresponse.indexOf("Successful") != -1) //判斷回傳內容是否有"Successful" 字串
      {
        Serial.println("Http GET Successful"); /*印出http get上傳成功*/
      } 
   else
      {
        Serial.println("Http GET Fail"); /*印出http get上傳失敗*/ 
      }
    Wifi.http_end(); //http get方式連線結束
  }
 else
 {
	//-----網路狀態沒有連線成功則進行
  	//-----重新連接網路---------
    initWiFi();              // 執行 WiFi 模組初始化與連線（定義於 TCP.h / BMC81M001.h）
      MacData = GetMAC() ; //取得 MAC 位址字串
    Serial.println("---MAC Address----"); // 分隔線，美觀用途
    Serial.println(MacData); // 印出取得 連接上的SSID熱點之後閘道器IP位址
    //------連接網路成功後，可以同樣處理雲端代理人連線程序
     //-----透過網路，使用http get方式連線------
	  Wifi.http_begin(ServerURL,ServerPort,connectstr);//透過網路，使用http get方式連線 
	//-----讀取http get連線回傳頁面資料------
	Wifi.http_get();//運行http get方式連線，並取得回傳頁面
   webresponse = Wifi.http_getString();//取得回傳頁面內容字串回傳到webresponse
    Serial.println(webresponse);//印出回傳頁面內容字串
   if (webresponse.indexOf("Successful") != -1) //判斷回傳內容是否有"Successful" 字串
      {
        Serial.println("Http GET Successful"); /*印出http get上傳成功*/
      } 
   else
      {
        Serial.println("Http GET Fail"); /*印出http get上傳失敗*/ 
      }
    Wifi.http_end(); //http get方式連線結束   
 }

}


