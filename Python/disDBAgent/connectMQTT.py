import paho.mqtt.client as mqtt  # 導入 paho.mqtt.client 套件，並將其命名為 mqtt

# ==================== MQTT 連線設定 ====================
# 以下設定 MQTT Broker（伺服器）的連線資訊
broker_address = "broker.emqx.io"  # MQTT Broker 的伺服器網址，這裡使用公開的 EMQX 測試伺服器
port = 1883  # MQTT Broker 的通訊埠號，1883 是 MQTT 協定的標準非加密埠
username = ""  # 連線時的使用者名稱（如果 Broker 需要驗證），此處為空表示無需驗證
password = ""  # 連線時的密碼（如果 Broker 需要驗證），此處為空表示無需驗證
topic = "/arduino/dht/#"  # 要訂閱的主題（Topic），# 是萬用字元，代表訂閱所有以 /arduino/dht/ 開頭的主題

# HTTP 請求的基礎 URL
BASE_URL = "http://iot.arduino.org.tw:8888/bmduino/dhtdata/dataadd.php"


# ==================== 回調函數定義 ====================
# 當客戶端成功連接到 MQTT Broker 時會呼叫此函數
def on_connect(client, userdata, flags, rc, properties=None):
    """
    連線成功時的回調函數
    參數說明：
    - client: MQTT 客戶端物件
    - userdata: 使用者自定義資料（此處未使用）
    - flags: 伺服器回傳的標誌
    - rc: 回傳碼（Return Code），表示連線結果
          0：連線成功
          1：連線失敗 - 不正確的協定版本
          2：連線失敗 - 無效的客戶端識別碼
          3：連線失敗 - 伺服器無法使用
          4：連線失敗 - 錯誤的使用者名稱或密碼
          5：連線失敗 - 未授權
    - properties: MQTT v5.0 的屬性參數（此處設定為 None 表示不使用）
    """
    print("已連線到 MQTT Broker，回傳碼：" + str(rc))  # 顯示連線結果

    # 連線成功後立即訂閱指定的主題
    if rc == 0:  # 只有當連線成功時才執行訂閱
        client.subscribe(topic)
        print("已訂閱主題：" + topic)
    else:
        print("連線失敗，無法訂閱主題")


# 當收到訂閱主題的訊息時會呼叫此函數
def on_message(client, userdata, msg):
    """
    收到訊息時的回調函數
    參數說明：
    - client: MQTT 客戶端物件
    - userdata: 使用者自定義資料（此處未使用）
    - msg: 接收到的訊息物件，包含以下屬性：
          msg.topic：訊息的主題（Topic）
          msg.payload：訊息的內容（原始位元組資料）
          msg.qos：訊息的服務品質等級
          msg.retain：訊息是否為保留訊息
    """
    # 將接收到的位元組資料解碼為字串並顯示
    print("收到訊息：")
    print("  主題：" + msg.topic)  # 顯示訊息來自哪個主題
    print("  內容：" + str(msg.payload.decode()))  # 將位元組解碼為字串並顯示
    print("-" * 30)  # 分隔線，讓輸出更清晰


# ==================== MQTT 客戶端初始化 ====================
# 建立 MQTT 客戶端物件
# 使用 VERSION2 的回調 API 版本以避免過時的警告
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# 設定連線時的使用者名稱和密碼（如果有的話）
client.username_pw_set(username, password)

# 將自定義的回調函數指定給客戶端物件
client.on_connect = on_connect  # 指定連線成功時要執行的函數
client.on_message = on_message  # 指定收到訊息時要執行的函數

# ==================== 建立連線與啟動 ====================
# 連線到 MQTT Broker 伺服器
# 參數說明：
# - broker_address: Broker 伺服器網址
# - port: 通訊埠號
# - keepalive=60: 保持連線的時間（秒），在此時間內若無通訊，會發送心跳封包維持連線
client.connect(broker_address, port, 60)
print("正在嘗試連線到 MQTT Broker...")

# 進入無窮迴圈模式，持續監聽來自 Broker 的訊息
# 這行程式會讓程式持續執行，直到手動停止或發生錯誤
# 在迴圈中，客戶端會：
# 1. 處理網路流量
# 2. 發送心跳封包維持連線
# 3. 呼叫相應的回調函數處理事件
client.loop_forever()

# 注意：程式執行到此處會一直停留，不會結束
# 若要停止程式，請按 Ctrl+C 中斷執行