// 定義 LED 腳位（常用的 LED 連接腳位為 8 號腳）
// 這個常數名稱 LedPin 可以讓程式更具可讀性，若之後更換腳位，只需改這裡即可。
#define LedPin 8

// setup() 是 Arduino 系統啟動後執行一次的初始化函式。
// 通常在這裡設定腳位模式（輸入或輸出）、初始化序列埠或感測器。
void setup() {
  // 設定 LedPin 腳位為輸出模式（OUTPUT）
  // 表示此腳位將輸出電壓訊號（HIGH = 5V, LOW = 0V）
  pinMode(LedPin, OUTPUT);
}

// loop() 函式會在 Arduino 啟動後，不斷重複執行（無限循環）
// 適合放入重複執行的任務，例如 LED 閃爍、感測器讀取等。
void loop() {
  // 將 LedPin 腳位輸出設定為 HIGH（高電位 = 5V）
  // 若腳位連接 LED，這會使 LED 亮起
  digitalWrite(LedPin, HIGH);

  // 延遲 1000 毫秒（即 1 秒）
  // 讓 LED 保持亮著 1 秒鐘
  delay(1000);

  // 將 LedPin 腳位輸出設定為 LOW（低電位 = 0V）
  // LED 會因為沒有電壓輸出而熄滅
  digitalWrite(LedPin, LOW);

  // 延遲 1000 毫秒（即 1 秒）
  // 讓 LED 保持熄滅 1 秒鐘
  delay(1000);
}
