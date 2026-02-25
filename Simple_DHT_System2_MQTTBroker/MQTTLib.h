// ================================================================
// 檔案名稱：MQTTLib.h
// 描述：MQTT 通訊函式庫
// 功能：提供與 MQTT Broker 連線、訊息發佈、JSON 資料處理等功能
//       用於將溫溼度感測資料發送至 MQTT 伺服器
// ================================================================

// ================================================================
// =============== 外部函式庫引入區 ===============
// ================================================================

// 引入 ArduinoJson 函式庫，用於處理 JSON 格式資料
#include <ArduinoJson.h>

// ========== 建立靜態 JSON 文件物件 =========
// 建立靜態 JSON 文件物件，配置 200 位元組記憶體空間
StaticJsonDocument<200> doc;  // 用於儲存感測資料的 JSON 結構

// ================================================================
// =============== MQTT Broker伺服器設定常數區 ===============
// ================================================================

// MQTT Client ID - 用於識別連線的客戶端
#define CLIENTLID  "aaaLiwJDBccDxULGxUwDiANEjs"   // 預設的固定 Client ID

// MQTT 認證資訊（此範例為匿名連線）
#define USERNAME  ""                              // 使用者名稱（空字串表示匿名）
#define PASSWORD  ""                              // 密碼（空字串表示無密碼）

// MQTT Broker 伺服器設定
#define MQTT_HOST "broker.emqx.io"                // MQTT 伺服器主機名稱
#define SERVER_PORT 1883                          // MQTT 通訊埠（非加密通訊）

// ================================================================
// =============== 全域變數宣告區 ===============
// ================================================================

// ------ 主題格式字串常數區 ------
// 發佈和訂閱使用的主題格式字串
// %s 為佔位符，會被 MAC 位址替換
const char* PubTop = "/arduino/dht/%s";   // 發佈主題格式
const char* SubTop = "/arduino/dht/%s";   // 訂閱主題格式

// 注意：已註解掉的舊版 JSON 格式
// const char* PrePayload = "{\"Device\":\"%s\",\"Temperature\":%4.3f,\"Humidity\":%4.1f}";

// ------ 動態字串緩衝區 ------
String TopicT;                    // 用於暫存主題字串
char PubTopicbuffer[200];         // 發佈主題緩衝區（字元陣列格式）
char SubTopicbuffer[200];         // 訂閱主題緩衝區（字元陣列格式）

String PayloadT;                  // 用於暫存承載資料（JSON 字串格式）
char Payloadbuffer[250];          // 承載資料緩衝區（處理特殊字元用）

char clintid[20];                 // MQTT Client ID 緩衝區

// ================================================================
// =============== 自訂函式宣告區 ===============
// ================================================================

// 注意：以下函式需在其他地方定義或實作
void printText(int x, int y, uint8_t *str);  // OLED 顯示函式（在 OledLib.h 中定義）

// MQTT 相關函式
void fillCID(String mm);                      // 產生動態 MQTT Client ID（基於 MAC 位址）
void fillTopic(String mm);                    // 填入對應 MAC 位址的主題字串
void insertBeforeChar(char *str, char target, char toInsert);  // 字串特殊字元處理
void fillPayload(String dev, float d1, float d2);  // 根據 MAC 位址、溫度、濕度產生 JSON 承載資料
void initMQTT();                             // 初始化 MQTT 伺服器連線
void MQTTPublish();                           // 發佈訊息到 MQTT Broker
void showStatusonOled(String ss);             // 在 OLED 上顯示狀態訊息

// ================================================================
// =============== 自訂函式實作區 ===============
// ================================================================

// ---------------------------------------------------------------
// 函式名稱：fillCID()
// 功能：產生動態 MQTT Client ID（基於裝置的 MAC 位址）
// 參數：
//   - mm: MAC 位址字串（如 "D8:BF:C0:12:34:56"）
// 傳回值：無
// 說明：
//   1. 在 MAC 位址前加上 "tw" 前綴
//   2. 儲存到全域變數 clintid 中
//   3. 序列埠輸出產生的 Client ID
// ---------------------------------------------------------------
void fillCID(String mm)
{
    // 產生基於 MAC 位址的隨機 Client ID
    // 組合方式：固定前綴 "tw" + MAC 位址
    
    // 設定前兩個字元為 "t" 和 "w"
    clintid[0] = 't';
    clintid[1] = 'w';
    
    // 將 MAC 位址複製到 clintid 陣列中（從第 2 個位置開始）
    // toCharArray() 將 String 轉換為 char 陣列
    mm.toCharArray(&clintid[2], mm.length() + 1);
    
    // 在字串結尾加上換行字元（實際應為終止字元 '\0'）
    // 注意：此處的 '\n' 應為 '\0' 更為正確
    clintid[2 + mm.length() + 1] = '\n';
    
    // 序列埠輸出產生的 Client ID
    Serial.print("Client ID:(");
    Serial.print(clintid);
    Serial.print(") \n");
}

// ---------------------------------------------------------------
// 函式名稱：fillTopic()
// 功能：填入對應 MAC 位址的主題字串
// 參數：
//   - mm: MAC 位址字串
// 傳回值：無
// 說明：
//   1. 將 MAC 位址填入主題格式字串的佔位符 %s 中
//   2. 產生發佈主題和訂閱主題
// ---------------------------------------------------------------
void fillTopic(String mm)
{
    // 產生發佈主題：將 MAC 位址填入 PubTop 格式字串
    // sprintf() 用於格式化字串
    sprintf(PubTopicbuffer, PubTop, mm.c_str());
    
    // 序列埠輸出發佈主題資訊（用於偵錯）
    Serial.print("Publish Topic Name:(");
    Serial.print(PubTopicbuffer);
    Serial.print("/");
    Serial.print(sizeof(PubTopicbuffer));      // 輸出緩衝區大小
    Serial.print("^");
    Serial.print(String(PubTopicbuffer));      // 轉換為 String 格式
    Serial.print("/");
    Serial.print(sizeof(String(PubTopicbuffer)));  // 輸出 String 物件大小
    Serial.print(") \n");
    
    // 產生訂閱主題：將 MAC 位址填入 SubTop 格式字串
    sprintf(SubTopicbuffer, SubTop, mm.c_str());
    
    // 序列埠輸出訂閱主題資訊
    Serial.print("Subscribe Topic Name:(");
    Serial.print(SubTopicbuffer);
    Serial.print(") \n");
}

// ---------------------------------------------------------------
// 函式名稱：insertBeforeChar()
// 功能：在特定字元前插入轉譯字元
// 參數：
//   - str: 要處理的字串（會直接修改）
//   - target: 目標字元（在此字元前插入轉義字元）
//   - toInsert: 要插入的轉義字元（通常是反斜線 '\'）
// 傳回值：無
// 說明：
//   1. 主要用於處理 JSON 字串中的特殊字元
//   2. 確保 JSON 字串符合格式要求
//   3. 例如：將 " 轉換為 \"，將 , 轉換為 \,
// ---------------------------------------------------------------
void insertBeforeChar(char *str, char target, char toInsert)
{
    int len = strlen(str);        // 取得原始字串長度
    uint8_t count = 0;            // 記錄已插入的字元數
    
    // 遍歷整個字串（考慮插入字元後的長度變化）
    for (int j = 0; j < len + count; j++) {
        // 如果找到目標字元
        if (str[j] == target) {
            count++;  // 增加插入計數
            
            // 將目標字元及其後的所有字元向後移動一位
            // 為要插入的轉義字元騰出空間
            for (int i = len + count; i >= j; i--) {
                str[i + 1] = str[i];
            }
            
            // 在目標字元前插入轉義字元
            str[j] = toInsert;
            
            // 跳過剛剛插入的字元，避免無限循環
            j++;
        }
    }
}

// ---------------------------------------------------------------
// 函式名稱：fillPayload()
// 功能：根據 MAC 位址、溫度、濕度產生 JSON 格式的資料內容
// 參數：
//   - dev: 裝置 MAC 位址
//   - d1: 溫度值（攝氏度）
//   - d2: 濕度值（百分比）
// 傳回值：無
// 說明：
//   1. 使用 ArduinoJson 函式庫建立 JSON 物件
//   2. 將資料填入 JSON 物件
//   3. 序列化為字串格式
//   4. 處理特殊字元以符合 MQTT 傳輸要求
// ---------------------------------------------------------------
void fillPayload(String dev, float d1, float d2)
{
    Serial.println("Fill Pay LOAD is Processing");
    
    // 舊版方法（使用 sprintf，已註解）
    // sprintf(Payloadbuffer, PrePayload, dev.c_str(), d1, d2);
    
    // ------ 新版方法：使用 ArduinoJson 函式庫 ------
    
    // 步驟1：將資料填入 JSON 文件物件
    doc["Device"] = dev;          // 加入裝置識別碼（MAC 位址）
    doc["Temperature"] = d1;      // 加入溫度值
    doc["Humidity"] = d2;         // 加入濕度值
    
    // 步驟2：將 JSON 物件序列化為字串
    // serializeJson() 將 JSON 物件轉換為字串格式
    serializeJson(doc, PayloadT);
    
    // ------ 特殊字元處理 ------
    
    // 步驟3：將 String 轉換為 char 陣列以進行字元處理
    PayloadT.toCharArray(Payloadbuffer, PayloadT.length() + 1);
    
    // 步驟4：處理 JSON 字串中的特殊字元
    // 在雙引號前插入反斜線（將 " 轉為 \"）
    insertBeforeChar(Payloadbuffer, '\"', '\\');
    
    // 在逗號前插入反斜線（將 , 轉為 \,）
    // 注意：此處理可能非必要，視 MQTT Broker 要求而定
    insertBeforeChar(Payloadbuffer, ',', '\\');
    
    // 步驟5：將處理後的字元陣列轉回 String
    PayloadT = String(Payloadbuffer);
    
    // 步驟6：序列埠輸出承載資料（用於偵錯）
    Serial.print("Sending:(");
    Serial.print(PayloadT);
    Serial.print(")\n");
}

// ---------------------------------------------------------------
// 函式名稱：initMQTT()
// 功能：初始化 MQTT 伺服器連線
// 參數：無
// 傳回值：無
// 流程：
//   1. 產生 Client ID 和主題字串
//   2. 顯示連線設定資訊
//   3. 嘗試連線到 MQTT Broker
//   4. 設定訂閱主題
// ---------------------------------------------------------------
void initMQTT()
{
    // 步驟1：產生 Client ID（基於 MAC 位址）
    fillCID(MacData);
    
    // 步驟2：產生主題字串（基於 MAC 位址）
    fillTopic(MacData);
    
    // 步驟3：顯示連線設定資訊（用於偵錯）
    Serial.print("clintid:(");
    Serial.print(clintid);
    Serial.print(")\n");
    
    Serial.print("USERNAME:(");
    Serial.print(USERNAME);
    Serial.print(")\n");
    
    Serial.print("PASSWORD:(");
    Serial.print(PASSWORD);
    Serial.print(")\n");
    
    Serial.print("MQTT_HOST:(");
    Serial.print(MQTT_HOST);
    Serial.print(")\n");
    
    Serial.print("SERVER_PORT:(");
    Serial.print(SERVER_PORT);
    Serial.print(")\n");
    
    Serial.println("Now Connect MQTT Broker.....");
    
    // 步驟4：嘗試連線到 MQTT Broker
    // 使用 Wifi 物件的 configMqtt() 方法進行連線
    if (Wifi.configMqtt(String(clintid), USERNAME, PASSWORD, MQTT_HOST, SERVER_PORT) != 0) {
        // 連線成功
        Serial.println("Connect to MQTT Server Successful");
    } else {
        // 連線失敗
        Serial.println("Connect to MQTT Server failed");
        
        // 進入無窮迴圈，停止程式執行
        // 注意：實際應用中應加入重試機制而非直接停止
        while(1);  // 永久迴圈，無法離開，代表終止程式
    }
    
    // 步驟5：設定訂閱主題
    // 訂閱與發佈相同的主題（用於接收回應或其他訊息）
    Wifi.setSubscribetopic(String(SubTopicbuffer));
    
    // 注意：以下為舊版程式碼，已註解但保留作為參考
    // mqttclient.setCallback(callback);  // 設定收到訂閱訊息時的回呼函式
}

// ---------------------------------------------------------------
// 函式名稱：MQTTPublish()
// 功能：發佈感測資料到 MQTT Broker伺服器
// 參數：無
// 傳回值：無
// 流程：
//   1. 產生 JSON 感測資料文件
//   2. 顯示發送資訊
//   3. 透過 WiFi 模組發送資料
//   4. 顯示發送結果狀態
// ---------------------------------------------------------------
void MQTTPublish()
{
    // 步驟1：產生 JSON 感測資料文件
    // 參數：MAC 位址、溫度值、濕度值
    fillPayload(MacData.c_str(), TValue, HValue);
    
    // 步驟2：顯示發送資訊（用於偵錯）
    Serial.print("Now payload:(");
    Serial.print(PubTopicbuffer);   // 顯示發佈主題
    Serial.print("==>");
    Serial.print(PayloadT);         // 顯示感測資料文件
    Serial.print(")\n");
    
    // 步驟3：透過 WiFi 模組發送資料到 MQTT Broker
    // Wifi.writeString() 將感測資料文件發佈到指定主題
    if (Wifi.writeString(PayloadT, String(PubTopicbuffer))) {
        // 發送成功
        Serial.println("Send String data sucess");  // 序列埠輸出成功訊息
        
        // 在 OLED 上顯示成功狀態
        showStatusonOled("MQTT OK");
        
        // 延遲一秒，避免過快發送
        delay(1000);
    } else {
        // 發送失敗
        showStatusonOled("MQTT Fail");  // 在 OLED 上顯示失敗狀態
    }
}

// ---------------------------------------------------------------
// 函式名稱：showStatusonOled()
// 功能：在OLED上顯示MQTT通訊狀態
// 參數：
//   - ss: 要顯示的狀態訊息字串
// 傳回值：無
// 說明：
//   1. 清除 OLED 第 6 行內容
//   2. 顯示新的狀態訊息
//   3. 序列埠輸出偵錯訊息
// ---------------------------------------------------------------
void showStatusonOled(String ss)
{
    // 步驟1：清除 OLED 第 6 行內容（顯示 9 個空格）
    printText(0, 6, "         ");
    
    // 步驟2：顯示新的狀態訊息
    printText(0, 6, ss);
    
    // 步驟3：序列埠輸出偵錯訊息
    Serial.print("Status:(");
    Serial.print(ss);
    Serial.print(")\n");
}

// ================================================================
// ===================== 使用注意事項 ============================
// ================================================================
/*
重要提醒：

1. MQTT Broker 選擇：
   - 預設使用公開的 EMQX Broker (broker.emqx.io)
   - 生產環境應使用私有或更穩定的 Broker
   - 可考慮使用具有認證機制的 Broker

2. 安全性考量：
   - 目前使用匿名連線，不建議用於生產環境
   - 應設定 USERNAME 和 PASSWORD 進行認證
   - 考慮使用 TLS/SSL 加密連線（埠號 8883）

3. Client ID 設計：
   - 目前使用 "tw" + MAC 位址的方式
   - Client ID 應具有唯一性，避免衝突
   - 可考慮加入時間戳記或隨機數

4. 主題設計：
   - 目前使用 /arduino/dht/{MAC} 格式
   - 可考慮分層設計，如：/location/device/type/{ID}
   - 避免使用特殊字元和過長的主題

5. JSON 格式：
   - 使用 ArduinoJson 函式庫處理 JSON
   - JSON 文件大小限制為 200 位元組
   - 可根據需求調整 StaticJsonDocument 的大小

6. 錯誤處理：
   - 連線失敗時程式會停止（while(1)）
   - 應加入重試機制和錯誤恢復
   - 可考慮加入看門狗定時器

7. 特殊字元處理：
   - insertBeforeChar() 函式處理 JSON 特殊字元
   - 某些 Broker 可能不需要此處理
   - 應根據實際需求調整

8. 記憶體使用：
   - 多個字串緩衝區會佔用 RAM
   - Arduino Uno 僅有 2KB RAM，需注意使用量
   - 可考慮使用 PROGMEM 儲存常數字串

9. 網路穩定性：
   - MQTT 連線可能因網路不穩中斷
   - 應加入 Keep Alive 機制
   - 可實現遺言（Last Will）功能

10. 效能考量：
    - 每 2 分鐘發送一次資料（主程式設定）
    - 可根據需求調整發送頻率
    - 避免過頻繁發送造成網路負擔
*/