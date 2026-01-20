import paho.mqtt.client as mqtt  # 導入 paho.mqtt.client 套件，並將其命名為 mqtt
# 使用 paho 套件，將 paho.mqtt.client 套件下連線的物件 import 進來並更名為 mqtt
import json #將json套件仔入

# 設定 MQTT Broker 伺服器詳細資訊
broker_address = "broker.emqx.io"  # 設定 MQTT Broker 伺服器網址
port = 1883  # 設定 MQTT Broker 伺服器通訊埠
username = ""  # 設定 MQTT Broker 伺服器登錄使用者名稱
password = ""  # 設定 MQTT Broker 伺服器登錄使用者密碼
topic = "/arduino/dht/#"  # 訂閱 MQTT Broker 伺服器主題

# MQTT get published data as following
# {
#   "Device": "E89F6DE8F3BC",
#   "Temperature": 24,
#   "Humidity": 77
# }

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

# 定義訂閱回調函數
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
    print("Data coming from "+msg.topic)  # 打印接收到的主題和消息內容
    # jsonStr = json.dumps(msg.payload, ensure_ascii=False, indent=4)
    print("Payload is :\n"+str(msg.payload))  # 打印接收到的主題和消息內容
    Payload = msg.payload.decode("utf-8")
    jsondata = json.loads(Payload)
    jsonStr = json.dumps(jsondata, ensure_ascii=False, indent=4)
    print(jsonStr)  # 印出jsonStr
    print("Devce:",jsondata["Device"])
    print("Temperature:",jsondata["Temperature"])
    print("Humidity:",jsondata["Humidity"])



#--
# 建立 MQTT 客戶端物件
# 使用 VERSION2 的回調 API 版本以避免過時的警告
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
#client = mqtt.Client(protocol=mqtt.MQTTv311, callback_api_version=5)
# 設定帳戶名稱和密碼
client.username_pw_set(username, password)

# 設定伺服器連線的回調函數和訂閱的回調函數
client.on_connect = on_connect  # 設定伺服器連線的回調函數
client.on_message = on_message  # 設定訂閱的回調函數

# 連接到 MQTT Broker 伺服器
client.connect(broker_address, port, 60)
# client.connect(broker_address, port, keepalive=60) 各個參數的意義如下：
# broker_address：需要連接的 MQTT broker 伺服器的地址，可以是 IP 地址或主機名稱。
# port：需要連接的 MQTT broker 伺服器的通訊埠，預設值為 1883，可以是其他通訊埠。
# keepalive：保持活動狀態的時間間隔（秒），預設為 60 秒。在這個時間內，如果客戶端沒有向 MQTT broker 伺服器發送任何消息，
# broker 伺服器會向客戶端發送一個 PING 消息來維持連接狀態。如果 keepalive 設置為 0，表示不啟用伺服器保持活動狀態功能。
# 簡單來說，這行程式碼的作用是使用指定的地址和端口，建立與 MQTT broker 的連接，並設置 60 秒的 keepalive 時間間隔。
# 這個連接可以用於發布和訂閱 MQTT 消息，並在完成後使用 client.disconnect() 方法關閉連接。

# 進入循環模式，等待 MQTT Broker 伺服器針對主題傳送之訊息後，接收訊息
client.loop_forever()
