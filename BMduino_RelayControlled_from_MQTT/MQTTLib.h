/*
MQTT 函式庫 (MQTTLib.h)
功能：提供 MQTT 通訊相關的函式和設定，用於連接 MQTT Broker 並處理主題訂閱與發布
*/

// ==================== 引入必要的函式庫 ====================

#include <ArduinoJson.h>  // 引入 Arduino JSON 函式庫，用於處理 JSON 格式的資料

// ==================== 全域變數與常數定義 ====================

// 建立靜態 JSON 文件物件，配置 200 位元組的記憶體空間
// StaticJsonDocument 用於建立固定大小的 JSON 文件，適合記憶體有限的嵌入式系統
StaticJsonDocument<200> doc; 

// ==================== MQTT Broker 伺服器設定 ====================

//----- 宣告 MQTT Broker 伺服器登錄資料 ----- 

#define CLIENTLID  "aaaLiwJDBccDxULGxUwDiANEjs"   // 預設的 MQTT 客戶端 ID（可被動態產生覆蓋）
#define USERNAME  ""                              // MQTT Broker 使用者名稱（留空表示匿名）
#define PASSWORD  ""                              // MQTT Broker 密碼（留空表示匿名）
#define MQTT_HOST "broker.emqx.io"                // MQTT Broker 主機地址（使用公開的 EMQX 伺服器）
#define SERVER_PORT 1883                          // MQTT 通訊埠（標準非加密埠）

// ==================== 主題與訊息緩衝區設定 ====================

//------ 格式化字串變數區 -------------------------
// 用於 MQTT 發布（Publish）和訂閱（Subscribe）的字串處理

// String topic ;                        // MQTT 主題變數（註解掉的備用定義）
String topic="/arduino/SW/%s" ;        // 預設的 MQTT 主題，使用萬用字元 # 訂閱所有相關主題

// 主題格式模板
const char* PubTop = "/arduino/SW/%s" ;
const char* SubTop = "/arduino/SW/#" ;
// 主題和訊息相關變數
String TopicT;                          // 主題字串暫存變數
char SubTopicbuffer[200];               // 訂閱主題字元陣列緩衝區（最大 200 字元）
char PubTopicbuffer[200];               // 發布主題字元陣列緩衝區（最大 200 字元）
String PayloadT;                        // 訊息內容（Payload）字串暫存變數
char Payloadbuffer[250];                // 訊息內容字元陣列緩衝區（最大 250 字元）
char clintid[20];                       // 客戶端 ID 字元陣列緩衝區（最大 20 字元）

// ==================== 自定義函式宣告區 ====================

/**
 * 產生動態 MQTT Broker ClientID
 * @param mm MAC 地址字串，用於產生唯一的客戶端 ID
 */
void fillCID(String mm);

/**
 * 填入對應 MAC Address 的主題
 * @param mm MAC 地址字串，用於產生對應的主題名稱
 */
void fillTopic(String mm);

/**
 * 在指定字元前插入新字元
 * @param str 目標字串（會直接修改）
 * @param target 目標字元，在此字元前插入新字元
 * @param toInsert 要插入的新字元
 * 說明：主要用於處理 AT 指令中需要轉義的雙引號
 */
void insertBeforeChar(char *str, char target, char toInsert);

/**
 * 初始化 MQTT 伺服器連線
 * 包含客戶端 ID 產生、主題設定和伺服器連線
 */
void initMQTT();

// ==================== 自定義函式主體區 ====================

// 序列通訊相關常數定義
#define DEB_CNT     80                 // 除錯計數器，延遲 50ms（可能用於等待回應）
#define RES_MAX_LENGTH 200             // 序列通訊緩衝區最大長度

// 序列通訊相關變數
String ReciveBuff;                     // 接收緩衝區字串
int ReciveBufflen;                     // 接收緩衝區長度
// String DATA_BUF ;                   // 資料緩衝區（註解掉的備用變數）

// ==================== 函式實作 ====================

//-------- 產生動態 MQTT Broker ClientID --------

/**
 * 產生動態 MQTT Broker ClientID
 * 函式功能：根據 MAC 地址產生唯一的客戶端 ID
 * 格式：以 "tw" 開頭，後面接上 MAC 地址
 * 
 * @param mm MAC 地址字串，例如 "E89F6DE8F3BC"
 * 
 * 範例：
 * 輸入：mm = "E89F6DE8F3BC"
 * 輸出：clintid = "twE89F6DE8F3BC"
 */
void fillCID(String mm) 
{
    // 產生基於 MAC 地址的隨機客戶端 ID
    // 組合客戶端 ID：前綴 "tw" + MAC 地址
    
    // 設定前綴字元
    clintid[0] = 't';  
    clintid[1] = 'w';  
    
    // 將 MAC 地址複製到字元陣列中，從索引 2 開始
    // toCharArray() 將 String 轉換為 char 陣列
    mm.toCharArray(&clintid[2], mm.length() + 1);
    
    // 在字串結尾添加換行字元（可能需要調整，通常 MQTT ID 不需要換行字元）
    clintid[2 + mm.length() + 1] = '\n';
    
    // 顯示產生的客戶端 ID
    Serial.print("客戶端 ID:(");
    Serial.print(clintid);
    Serial.print(") \n");
}

//------ 填入對應 MAC Address 的主題 --------

/**
 * 填入對應 MAC Address 的主題
 * 函式功能：根據 MAC 地址產生發布和訂閱的主題名稱
 * 
 * @param mm MAC 地址字串
 * 
 * 發布主題格式："/arduino/dht/[MAC地址]"
 * 訂閱主題格式："/arduino/dht/#"（萬用字元，訂閱所有相關主題）
 * 
 * 範例：
 * 輸入：mm = "E89F6DE8F3BC"
 * 輸出：發布主題 = "/arduino/dht/E89F6DE8F3BC"
 *       訂閱主題 = "/arduino/dht/#"
 */
void fillTopic(String mm) 
{
    // 使用 sprintf 格式化字串，產生發布主題
    // %s 會被 MAC 地址取代
    sprintf(PubTopicbuffer, PubTop, mm.c_str());
    
    // 顯示發布主題資訊（用於除錯）
    Serial.print("發布主題名稱:(");
    Serial.print(PubTopicbuffer);
    Serial.print("/");
    Serial.print(sizeof(PubTopicbuffer));   // 顯示緩衝區大小
    Serial.print("^");
    Serial.print(String(PubTopicbuffer));   // 轉換為 String 顯示
    Serial.print("/");
    Serial.print(sizeof(String(PubTopicbuffer)));  // 顯示 String 物件大小
    Serial.print(") \n");
    
    // 產生訂閱主題
    sprintf(SubTopicbuffer, SubTop, mm.c_str());
    
    // 顯示訂閱主題資訊（用於除錯）
    Serial.print("訂閱主題名稱:(");
    Serial.print(SubTopicbuffer);
    Serial.print("/");
    Serial.print(SubTop);                   // 顯示原始主題格式
    Serial.print(") \n");
}

//----- 處理字串中特定字元前插入轉義字元 -----

/**
 * 在指定字元前插入新字元
 * 函式功能：處理 AT 指令中需要轉義的字元，例如在雙引號前插入反斜線
 * 
 * @param str 目標字串（直接修改原字串）
 * @param target 目標字元，在此字元前插入新字元
 * @param toInsert 要插入的新字元
 * 
 * 工作原理：
 * 1. 遍歷字串中的每個字元
 * 2. 找到目標字元時，將後續字元向後移動一位
 * 3. 在目標字元位置插入新字元
 * 4. 跳過新插入的字元，繼續處理
 * 
 * 範例：
 * 輸入：str = "test"data"，target = '"'，toInsert = '\'
 * 輸出：str = "test\"data"
 */
void insertBeforeChar(char *str, char target, char toInsert) 
{
    int len = strlen(str);            // 取得原始字串長度
    uint8_t count = 0;                // 計數器，記錄插入的字元數量
    
    // 遍歷字串（長度會因插入字元而變化）
    for (int j = 0; j < len + count; j++) {
        // 檢查目前字元是否為目標字元
        if (str[j] == target) {
            count++;                   // 增加插入計數
            
            // 從字串結尾開始，將字元向後移動一位，騰出空間
            for (int i = len + count; i >= j; i--) {
                str[i + 1] = str[i];
            }
            
            str[j] = toInsert;         // 插入新字元
            j++;                       // 跳過剛插入的字元，避免無限循環
        }
    }
}

/**
 * 初始化 MQTT 伺服器連線
 * 函式功能：設定 MQTT 客戶端，連接到 MQTT Broker，並處理主題訂閱
 * 
 * 執行步驟：
 * 1. 產生客戶端 ID
 * 2. 設定發布和訂閱主題
 * 3. 顯示連線設定資訊
 * 4. 嘗試連接到 MQTT Broker
 * 5. 設定訂閱主題
 * 
 * 注意：此函式假設 MacData 變數已定義並包含 MAC 地址
 */
void initMQTT()
{
    // 步驟 1：產生基於 MAC 地址的客戶端 ID
    fillCID(MacData);
    
    // 步驟 2：設定對應的主題
    fillTopic(MacData);
    
    // 步驟 3：顯示所有連線設定資訊（用於除錯）
    Serial.print("客戶端 ID:(");
    Serial.print(clintid);
    Serial.print(")\n");
    
    Serial.print("使用者名稱:(");
    Serial.print(USERNAME);
    Serial.print(")\n");
    
    Serial.print("密碼:(");
    Serial.print(PASSWORD);
    Serial.print(")\n");
    
    Serial.print("MQTT 主機:(");
    Serial.print(MQTT_HOST);
    Serial.print(")\n");
    
    Serial.print("伺服器埠號:(");
    Serial.print(SERVER_PORT);
    Serial.print(")\n");
    
    Serial.println("正在連接到 MQTT Broker.....");
    
    // 步驟 4：嘗試連接到 MQTT Broker
    // 使用 WiFi 模組的 configMqtt 函式進行連線
    // 參數說明：客戶端 ID、使用者名稱、密碼、主機地址、埠號
    if (Wifi.configMqtt(String(clintid), USERNAME, PASSWORD, MQTT_HOST, SERVER_PORT) != 0)
    {
        // 連線成功
        Serial.println("成功連接到 MQTT 伺服器");
    }
    else 
    {
        // 連線失敗
        Serial.println("連接到 MQTT 伺服器失敗");
        
        // 進入永久迴圈，停止程式執行
        // 這是一個嚴重的錯誤處理方式，實際應用中可能需要重試機制
        while(1);  // 永久迴圈，無法離開，代表終止程式
    }
    
    // 步驟 5：設定訂閱主題
    // 設定 WiFi 模組訂閱指定的主題
    Wifi.setSubscribetopic(SubTop);
    
    // 以下為註解掉的程式碼，可能在其他版本中使用
    /*
    // 連接 MQTT Server ，伺服器名稱: MQTTServer，伺服器埠號: MQTTPort
    // mq.tongxinmao.com:18832
    // mqttclient.setCallback(callback);
    // 設定 MQTT Server，當訂閱的主題有訊息時，呼叫的 callback 函數
    */
}

// ==================== 被註解掉的函式 ====================

/*
以下函式被註解掉，但保留了原始程式碼以供參考：

void fillPayload(String dev, float d1, float d2)
{
    // 根據 MAC 地址、溫度和濕度填入訊息內容
    Serial.println("正在處理訊息內容填充...");
    
    // 使用 ArduinoJson 函式庫建立 JSON 物件
    doc["Device"] = dev;           // 裝置 MAC 地址
    doc["Temperature"] = d1;       // 溫度值
    doc["Humidity"] = d2;          // 濕度值
    
    // 將 JSON 物件序列化為字串
    serializeJson(doc, PayloadT);
    
    // 將 String 轉換為 char 陣列以便處理
    PayloadT.toCharArray(Payloadbuffer, PayloadT.length() + 1);
    
    // 在雙引號前插入反斜線進行轉義（AT 指令需要）
    insertBeforeChar(Payloadbuffer, '\"', '\\');
    
    // 在逗號前插入反斜線進行轉義（AT 指令需要）
    insertBeforeChar(Payloadbuffer, ',', '\\');
    
    // 將處理後的字元陣列轉回 String
    PayloadT = String(Payloadbuffer);
    
    // 顯示要發送的訊息內容
    Serial.print("正在發送:(");
    Serial.print(PayloadT);
    Serial.print(")\n");
}
*/

// ==================== 使用注意事項 ====================

/*
重要提醒：

1. 記憶體管理：
   - StaticJsonDocument<200> 使用靜態記憶體分配
   - 確保 JSON 文件不會超過 200 位元組
   - 在記憶體有限的 Arduino 上要特別注意

2. 字串緩衝區：
   - 所有緩衝區都有固定大小
   - SubTopicbuffer: 200 字元
   - PubTopicbuffer: 200 字元
   - Payloadbuffer: 250 字元
   - clintid: 20 字元
   - 確保輸入資料不會超過這些限制

3. 錯誤處理：
   - MQTT 連線失敗會進入永久迴圈
   - 實際應用中可能需要加入重試機制
   - 考慮加入超時處理

4. 相依性：
   - 需要 ArduinoJson 函式庫
   - 假設存在 Wifi 物件（可能來自 TCP.h）
   - 假設存在 MacData 全域變數

5. 安全性：
   - 公開的 MQTT Broker 可能不安全
   - 考慮使用加密連線（MQTTS，埠號 8883）
   - 避免在公開專案中暴露敏感資訊
*/

// ==================== 函式使用範例 ====================

/*
使用範例：

1. 在 setup() 函式中初始化：
   void setup() {
       // ... 其他初始化 ...
       initMQTT();  // 初始化 MQTT 連線
   }

2. 發送資料時（使用註解掉的 fillPayload 函式）：
   // 建立 JSON 訊息
   String deviceID = "E89F6DE8F3BC";
   float temperature = 25.5;
   float humidity = 60.0;
   
   // 填入訊息內容
   fillPayload(deviceID, temperature, humidity);
   
   // 發送訊息（需實作對應的發送函式）

3. 接收訊息：
   // 需要實作 callback 函式處理接收到的訊息
   // 並在 initMQTT() 中設定回調函式
*/

// ==================== 版本紀錄 ====================

/*
版本紀錄：
V1.0 - 基本 MQTT 連線功能
V2.0 - 加入 JSON 訊息處理
V3.0 - 優化錯誤處理和除錯輸出

注意：此檔案為標頭檔（.h），應被其他 Arduino 程式包含使用
*/