/*
程式說明重點：

#define LedPin 8
這是前置處理指令，代表在編譯程式前，會將所有的 LedPin 文字替換成 8。
優點是程式可讀性高、修改方便。

pinMode(LedPin, OUTPUT)
設定腳位模式：

OUTPUT：此腳位輸出電壓訊號。

INPUT：此腳位讀取外部訊號。

digitalWrite(LedPin, HIGH) / digitalWrite(LedPin, LOW)
控制輸出電位：

HIGH → 腳位輸出高電位（通常為 +5V），LED 亮。

LOW → 腳位輸出低電位（0V），LED 熄。

delay(1000)
暫停程式執行 1000 毫秒（1 秒）。
Arduino 內建函式，單位是毫秒（ms）。

loop() 無限循環機制
Arduino 會自動反覆執行 loop() 內的程式碼，形成 LED 每秒亮滅一次的效果。

💡執行結果：

此程式會讓接在 D8 腳位 的 LED 不斷地「亮一秒 → 滅一秒 → 再亮一秒 → 滅一秒」，
形成一個穩定的閃爍效果（Blink），這是 Arduino 初學者常見的「Hello World」範例

*/