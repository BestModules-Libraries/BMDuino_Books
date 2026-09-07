#include "DHTLib.h" //引入溫溼度感測模組自訂模組

// ------- 自定義函式宣告區 -----------
void initSensor();                 // 初始化所有感測模組
void initAll();                    // 初始化整體系統


void setup() 
{
     initAll();	// 初始化整體系統
 
    // 顯示裝置的基本資訊：序號 SN、產品 ID、韌體版本
    ShowDHTInformation();   //印出溫溼度感測器產品所有資訊
    delay(2000);                // 延遲 2 秒，讓使用者有時間閱讀裝置資訊
}

void loop() 
{
    // 從感測器讀取濕度數值並顯示
    HValue = readTemperature() ;// 讀取溫度值
    Serial.print("Humidity : ");
    Serial.print(HValue);       // 顯示濕度值
    Serial.print(" %    ");                  // 顯示濕度的單位 %

    // 從感測器讀取溫度數值並顯示
    TValue = readTemperature(); // 讀取溫度值
    Serial.print("Temperature : ");
    Serial.print(TValue);    // 顯示溫度值
    Serial.println(" °C ");                  // 顯示溫度單位 °C

    delay(2000);                // 每隔 2 秒讀取並更新一次溫濕度資料
}

//---------------  自訂函式區 ------------------
// ------------------ 系統初始化區 ------------------

// 初始化所有感測模組
void initSensor()	// 初始化所有感測模組
{
  initDHT() ;// 初始化溫溼度感測器，啟動 I2C 通訊
  delay(2000);   // 延遲 2 秒，等待顯示模組穩定
}

// 初始化整體系統
void initAll()	// 初始化整體系統
{
  Serial.begin(9600);  // 啟動序列埠，速率 9600 bps
  Serial.println("System Start.....") ; //印出"System Start....." 
  initSensor(); 
}







