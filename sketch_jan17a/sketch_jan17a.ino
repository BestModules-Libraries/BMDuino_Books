
// 定義連接藍牙模組的序列埠
 // 接收腳, 傳送腳
char val;  // 儲存接收資料的變數

void setup() {
  Serial.begin(9600);   // 與電腦序列埠連線
  Serial.println("BT is ready!");

  // 設定藍牙模組的連線速率
  // 如果是HC-05，請改成9600
  Serial2.begin(9600);
}

void loop() {

  // 若收到藍牙模組的資料，則送到「序列埠監控視窗」
  if (Serial2.available() >0 ) 
    {
      val = Serial2.read();
      Serial.print(val);
    }

  // 若收到「序列埠監控視窗」的資料，則送到藍牙模組
  if (Serial.available() > 0) 
  {
    val = Serial.read();
    Serial2.write(val);
  }
}
