/*************************************************
  File:             write.ino
  Description:      本模組連接至手機 APP "BLEDemo"，按下 APP 上的按鍵，
                    模組接收按鍵並將其列印至序列埠監控視窗，
                    然後發送相應資料至 APP，使 APP 上的 LED 燈亮起
  Note:             
  Operation:        
**************************************************/

#include <BM7701-00-1.h>

// 定義 BLE 模組的串列通訊介面（根據硬體連接選擇其中一種）
//BM7701_00_1       BC7701(2, 3); // 使用軟體串列，接腳 rxPin=2, txPin=3，若未使用請註解此行
//BM7701_00_1     BC7701(&Serial1); // 使用硬體串列 Serial1，若在 BMduino 上使用請取消註解
BM7701_00_1     BC7701(&Serial2); // 使用硬體串列 Serial2，若在 BMduino 上使用請取消註解
//BM7701_00_1     BC7701(&Serial3); // 使用硬體串列 Serial3，若在 BMduino 上使用請取消註解
//BM7701_00_1     BC7701(&Serial4); // 使用硬體串列 Serial4，若在 BMduino 上使用請取消註解

// 定義 BLE 參數
#define TX_POWER     0x0F                   // 發射功率設定
#define XTAL_CLOAD   0x04                   // 16MHz 晶體負載設定
#define ADV_MIN      100                    // 廣播間隔最小值（單位：ms）
#define ADV_MAX      100                    // 廣播間隔最大值（單位：ms）
#define CON_MIN      30                     // 連接間隔最小值（單位：ms）
#define CON_MAX      30                     // 連接間隔最大值（單位：ms）
#define CON_LATENCY  00                     // 連接延遲設定
#define CON_TIMEOUT  300                    // 連接超時設定（單位：ms）

uint8_t BDAddress[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; // 裝置藍牙位址
uint8_t BDName[] = {'B', 'M', 'C', '7', '7', 'M', '0', '0', '1'}; // 裝置名稱
uint8_t Adata[] = {0x02, 0x01, 0x06};      // 廣播資料
uint8_t Sdata[] = {0x03, 0x02, 0x0f, 0x18}; // 掃描回應資料

/////////////////////////////////////////////////////////////////////////////////////////

// 按鍵相關定義
#define BUTTON_CONSISTENCY_DURATION    6    // 按鍵一致性檢測時間單位
#define BUTTON_REPEAT1_DURATION       (600 / BUTTON_CONSISTENCY_DURATION) // 第一次重複延遲
#define BUTTON_REPEAT2_DURATION       (150 / BUTTON_CONSISTENCY_DURATION) // 第二次重複延遲
#define INVERT_TIME                   500   // 反轉時間（未使用）

// 狀態變數
bool board_connect = false;    // 藍牙連接狀態
bool board_receive = false;    // 資料接收標誌
bool board_conIntv = false;    // 連接間隔設定標誌
uint8_t Status;                // BLE 狀態碼
uint8_t flag = 0;              // 發送資料標誌
uint8_t count = 0;             // 按鍵狀態計數
uint8_t sel = 1;               // 初始化步驟選擇
uint8_t receiveBuf[256] = {0}; // 接收資料緩衝區
KEY_MESSAGE Keymessage;        // 按鍵訊息結構

void setup() {
  delay(60); // 上電後延遲 60ms，此期間不能發送命令
  
  Serial.begin(9600);           // 初始化序列埠用於除錯輸出
  BC7701.begin(BAUD_115200);    // 初始化 BLE 模組通訊波特率為 115200
  
  // BLE 模組初始化流程，依序執行各項設定
  while (sel != 10) 
  {
    switch (sel) {
      case 1: // 設定藍牙位址
        if (BC7701.setAddress(BDAddress) == true) sel++;
        else sel = 0xFF; 
        break;
      case 2: // 設定藍牙名稱（最長 31 個字元）
        if (BC7701.setName(sizeof(BDName), BDName) == true) sel++;
        else sel = 0xFF; 
        break;
      case 3: // 設定廣播間隔（單位為 0.625ms）
        if (BC7701.setAdvIntv(ADV_MIN / 0.625, ADV_MAX / 0.625, 7) == true) sel++;
        else sel = 0xFF; 
        break;
      case 4: // 設定廣播資料（包含名稱）
        if (BC7701.setAdvData(APPEND_NAME, sizeof(Adata), Adata) == true) sel++;
        else sel = 0xFF; 
        break;
      case 5: // 設定掃描回應資料
        if (BC7701.setScanData(sizeof(Sdata), Sdata) == true) sel++;
        else sel = 0xFF; 
        break;
      case 6: // 設定發射功率
        if (BC7701.setTXpower(TX_POWER) == true) sel++;
        else sel = 0xFF; 
        break;
      case 7: // 設定晶體負載
        if (BC7701.setCrystalOffset(XTAL_CLOAD) == true) sel++;
        else sel = 0xFF; 
        break;
      case 8: // 設定特徵：自動發送狀態
        if (BC7701.setFeature(FEATURE_DIR, AUTO_SEND_SATUS) == true) sel++;
        else sel = 0xFF; 
        break;
      case 9: // 開啟廣播
        if (BC7701.setAdvCtrl(ENABLE) == true) sel++;
        else sel = 0xFF;
        break;
      case 0xFF: // 設定失敗，點亮板載 LED（通常接在腳位 13）
        digitalWrite(13, HIGH);
        break;
    }

  }
      Serial.print("Step:(") ;
    Serial.print(sel) ;
    Serial.print(")\n") ;
  delay(650); // 開啟廣播後延遲 650ms，此期間不能發送命令
}

void loop() {
  // 讀取並處理 BLE 狀態
  Status = bleProcess();
  
  if (Status) {
    switch (Status) {
      case API_CONNECTED: // 藍牙已連接
        if (board_connect == false) {
          board_connect = true;
          board_receive = false; // 重置接收標誌
        }
        break;
      case API_DISCONNECTED: // 藍牙已斷開
        board_connect = false;
        board_receive = false;
        board_conIntv = false; // 重置連接間隔設定標誌
        break;
      case DATA_RECEIVED: // 接收到資料
        if (board_connect == true) {
          digitalWrite(13, LOW); // 點亮板載 LED 表示收到資料
          board_receive = true;  // 設定接收標誌
        }
        break;
      case API_ERROR: // BLE 通訊錯誤
        digitalWrite(13, HIGH); // 點亮板載 LED 表示錯誤
        break;
    }
  }
  
  // 如果已連接藍牙
  if (board_connect == true) {
    // 設定連接間隔（僅在初次連接時設定一次）
    if (board_conIntv == false) {
      BC7701.wakeUp(); // 喚醒 BLE 模組
      delay(30);
      if (BC7701.setConnIntv(CON_MIN / 1.25, CON_MAX / 1.25, CON_LATENCY, CON_TIMEOUT) == true) {
        board_conIntv = true; // 設定成功
      }
    }
    
    // 如果有接收到資料
    if (board_receive == true) 
    {
      board_receive = false; // 重置接收標誌
      
      // // 檢查是否為按鍵資料（指令 0xB0）
      // if (receiveBuf[3] == 0xB0) {
      //   switch (receiveBuf[4]) {
      //     case 0x11:
      //       count = 1;
      //       Serial.println("KEY1 icon Pushed"); // 序列埠輸出：KEY1 被按下
      //       break;
      //     case 0x10:
      //       count = 2;
      //       Serial.println("KEY1 icon Released"); // 序列埠輸出：KEY1 被釋放
      //       break;
      //     case 0x22:
      //       count = 1;
      //       Serial.println("KEY2 icon Pushed"); // 序列埠輸出：KEY2 被按下
      //       break;
      //     case 0x20:
      //       count = 2;
      //       Serial.println("KEY2 icon Released"); // 序列埠輸出：KEY2 被釋放
      //       break;
      //     case 0x44:
      //       count = 1;
      //       Serial.println("KEY3 icon Pushed"); // 序列埠輸出：KEY3 被按下
      //       break;
      //     case 0x40:
      //       count = 2;
      //       Serial.println("KEY3 icon Released"); // 序列埠輸出：KEY3 被釋放
      //       break;
      //   }
        
      //   // 處理按鍵釋放事件，並回傳資料給 APP
      //   if (receiveBuf[4] != 0 && count == 2 && flag == 0) {
      //     // 第一種資料格式處理
      //     Keymessage.key = receiveBuf[4] >> 4;
      //     Keymessage.key += receiveBuf[4];
      //     Keymessage.serial++;
      //     Keymessage.checksum = 0xB1 ^ Keymessage.key ^ Keymessage.serial;
      //     BC7701.writeData((uint8_t*)&Keymessage, 3); // 發送 3 位元組資料
      //     flag = 1;
      //     receiveBuf[4] = 0; // 清除按鍵值
      //   }
        
      //   if (receiveBuf[4] != 0 && count == 2 && flag == 1) {
      //     // 第二種資料格式處理
      //     Keymessage.key = receiveBuf[4];
      //     Keymessage.serial++;
      //     Keymessage.checksum = 0xB1 ^ Keymessage.key ^ Keymessage.serial;
      //     BC7701.writeData((uint8_t*)&Keymessage, 3); // 發送 3 位元組資料
      //     flag = 0;
      //     receiveBuf[4] = 0; // 清除按鍵值
      //   }
      // }
    }
  }
}

/**********************************************************
  Description:    判斷 BLE 狀態
  Parameters:     無
  Return:         API_CONNECTED    - 已連接
                  API_DISCONNECTED - 已斷開
                  DATA_RECEIVED    - 收到資料
                  API_ERROR        - 通訊錯誤
  Others:         無
**********************************************************/
uint8_t bleProcess() {
  uint8_t st = 0x00;    // 狀態碼
  uint8_t lenth = 0;    // 接收資料長度
  
  // 從 BLE 模組讀取資料
  if (BC7701.readData(receiveBuf, lenth)) {
    switch (receiveBuf[1]) {
      case 0x00: // 狀態回報
        if (receiveBuf[0] == 0x00) 
        {
          if ((receiveBuf[3] & 0x01) == 0x01) 
          {
            st = API_CONNECTED; // 連接狀態位元為 1
          } 
          else 
          {
            st = API_DISCONNECTED; // 連接狀態位元為 0
          }
        }
        break;
      case 0xF2: // 資料接收
        if ((receiveBuf[0] == 0x00) && (receiveBuf[2] == 0xFF)) 
           {
          st = DATA_RECEIVED;
        }
        break;
      default: 
        break;
    }
  } else {
    st = API_ERROR; // 讀取資料失敗
  }
  
  return st;
}