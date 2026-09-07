/*
說明重點摘要：
區塊	功能	說明
全域變數區	儲存網路資訊	SSIDData, IPData, MacData 作為全域變數方便跨函式使用
函式庫引用區	引入模組功能	包含共用函式、WiFi / TCP 控制邏輯與 String 操作
initSensor()	初始化感測模組	若未連接感測器則留空，保留擴充介面
initAll()	系統初始化	啟動序列埠、呼叫感測器初始化
INITtWIFI()	WiFi 啟動	負責啟動 WiFi 模組並讀取 SSID、IP、MAC
setup()	啟動階段	呼叫初始化相關函式並顯示提示訊息
loop()	主迴圈	目前空白，方便日後擴充主邏輯


*/