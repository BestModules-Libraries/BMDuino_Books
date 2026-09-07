"""
MQTT 到 RESTful API 轉接器
功能：訂閱 MQTT 主題，接收感測器數據，轉發到 HTTP RESTful API
作者：自動生成繁體中文註解版本
日期：2024-11-25
"""

# ==================== 導入必要的套件 ====================
import paho.mqtt.client as mqtt  # 導入 MQTT 客戶端套件，用於 MQTT 通訊協定
import json  # 導入 JSON 套件，用於解析和處理 JSON 格式數據
import requests  # 導入 HTTP 請求套件，用於發送 HTTP GET 請求到伺服器

# ==================== MQTT 連線設定 ====================
# 設定 MQTT Broker（伺服器）的詳細資訊
broker_address = "broker.emqx.io"  # MQTT Broker 伺服器的網址，使用公開的 EMQX 測試伺服器
port = 1883  # MQTT 通訊埠號，1883 是 MQTT 協定的標準非加密埠
username = ""  # 連線時的使用者名稱（如果 Broker 需要驗證），此處為空表示無需驗證
password = ""  # 連線時的密碼（如果 Broker 需要驗證），此處為空表示無需驗證
topic = "/arduino/dht/#"  # 要訂閱的主題（Topic），# 是萬用字元，代表訂閱所有以 /arduino/dht/ 開頭的主題

# ==================== HTTP API 設定 ====================
# 設定要轉發數據的 HTTP RESTful API 端點
BASE_URL = "http://iot.arduino.org.tw:8888/bmduino/dhtdata/dataadd.php"  # HTTP API 的基礎 URL


# ==================== 預期接收的數據格式說明 ====================
# MQTT 發布的數據預期格式如下：
# {
#   "Device": "E89F6DE8F3BC",  # 裝置的 MAC 地址或唯一識別碼
#   "Temperature": 24,          # 溫度值（攝氏度）
#   "Humidity": 77              # 濕度值（百分比）
# }

# ==================== 回調函數定義區塊 ====================

def on_connect(client, userdata, flags, rc, properties=None):
    """
    當 MQTT 客戶端成功連接到 Broker 時會自動呼叫此函數
    這個函數是一個回調函數（callback），由 MQTT 客戶端在連線事件發生時呼叫

    參數說明：
    - client: MQTT 客戶端物件，代表當前的 MQTT 客戶端
    - userdata: 使用者自定義資料，此處未使用
    - flags: 伺服器回傳的連線標誌，包含連線的相關資訊
    - rc: 回傳碼（Return Code），表示連線的結果狀態
          0：連線成功
          1：連線失敗 - 不正確的協定版本
          2：連線失敗 - 無效的客戶端識別碼
          3：連線失敗 - 伺服器無法使用
          4：連線失敗 - 錯誤的使用者名稱或密碼
          5：連線失敗 - 未授權
    - properties: MQTT v5.0 的屬性參數，此處設定為 None 表示使用 MQTT v3.1.1
    """
    # 顯示連線結果到控制台
    print("已連線到 MQTT Broker，回傳碼：" + str(rc))

    # 根據回傳碼判斷連線是否成功
    if rc == 0:  # rc == 0 表示連線成功
        # 連線成功後立即訂閱指定的主題
        client.subscribe(topic)
        print("已訂閱主題：" + topic)
    else:  # rc != 0 表示連線失敗
        print("連線失敗，無法訂閱主題")


def on_message(client, userdata, msg):
    """
    當收到訂閱主題的訊息時會自動呼叫此函數
    這個函數處理從 MQTT Broker 接收到的所有訊息

    參數說明：
    - client: MQTT 客戶端物件
    - userdata: 使用者自定義資料（此處未使用）
    - msg: 接收到的訊息物件，包含以下屬性：
          msg.topic：訊息的主題（Topic），表示訊息來自哪個主題
          msg.payload：訊息的內容（原始位元組資料），需要解碼才能讀取
          msg.qos：訊息的服務品質等級（Quality of Service）
          msg.retain：訊息是否為保留訊息（Broker 會保存並發送給新訂閱者）
    """
    # 顯示訊息來源的主題
    print("接收到來自主題的數據: " + msg.topic)

    # 顯示原始 payload 數據（位元組格式）
    print("原始數據 (Payload):\n" + str(msg.payload))

    # 使用 try-except 區塊處理可能發生的各種錯誤
    try:
        # ==================== 步驟 1：解碼位元組數據 ====================
        # 將接收到的位元組資料解碼為 UTF-8 格式的字串
        Payload = msg.payload.decode("utf-8")

        # ==================== 步驟 2：解析 JSON 數據 ====================
        # 將 JSON 格式的字串解析為 Python 字典物件
        jsondata = json.loads(Payload)

        # ==================== 步驟 3：格式化並顯示 JSON 數據 ====================
        # 將字典轉換為格式化的 JSON 字串（方便閱讀）
        jsonStr = json.dumps(jsondata, ensure_ascii=False, indent=4)
        print("格式化後的 JSON 數據:")
        print(jsonStr)

        # ==================== 步驟 4：提取所需的數據欄位 ====================
        # 從 JSON 數據中提取裝置識別碼、溫度和濕度值
        # 使用 .get() 方法可以安全地獲取值，避免鍵不存在的錯誤
        device = jsondata.get("Device")  # 裝置 MAC 地址
        temperature = jsondata.get("Temperature")  # 溫度值
        humidity = jsondata.get("Humidity")  # 濕度值

        # 顯示提取的數據值
        print("裝置識別碼 (Device):", device)
        print("溫度值 (Temperature):", temperature)
        print("濕度值 (Humidity):", humidity)

        # ==================== 步驟 5：檢查數據完整性 ====================
        # 確認所有必要的數據欄位都存在且不為 None
        if device and temperature is not None and humidity is not None:
            # ==================== 步驟 6：構建 HTTP GET 請求參數 ====================
            # 建立查詢參數字串，將數據轉換為 URL 參數格式
            params = {
                "MAC": device,  # 裝置 MAC 地址
                "T": str(temperature),  # 溫度值（轉為字串）
                "H": str(humidity)  # 濕度值（轉為字串）
            }

            # ==================== 步驟 7：發送 HTTP GET 請求 ====================
            # 使用 requests 套件發送 HTTP GET 請求到指定的 API 端點，就是http://iot.arduino.org.tw:8888/bmduino/dhtdata/dataadd.php
            response = requests.get(BASE_URL, params=params)

            # ==================== 步驟 8：檢查 HTTP 響應狀態 ====================
            # 檢查伺服器回傳的狀態碼，200 表示請求成功
            if response.status_code == 200:
                print(f"HTTP 請求成功！狀態碼: {response.status_code}")
                print(f"伺服器響應內容: {response.text}")
            else:
                # 狀態碼不是 200，表示請求失敗
                print(f"HTTP 請求失敗！狀態碼: {response.status_code}")
                print(f"錯誤信息: {response.text}")
        else:
            # 數據不完整，缺少必要的欄位
            print("錯誤：JSON 數據中缺少必要的字段")

    # ==================== 例外處理區塊 ====================
    # 處理 JSON 解析過程中可能發生的錯誤
    except json.JSONDecodeError as e:
        print(f"JSON 解析錯誤: {e}")
        print("可能的原因：數據不是有效的 JSON 格式")

    # 處理字串解碼過程中可能發生的錯誤
    except UnicodeDecodeError as e:
        print(f"解碼錯誤: {e}")
        print("可能的原因：數據不是有效的 UTF-8 編碼")

    # 處理 HTTP 請求過程中可能發生的網路錯誤
    except requests.exceptions.RequestException as e:
        print(f"HTTP 請求錯誤: {e}")
        print("可能的原因：網路連線問題、伺服器無回應等")

    # 處理其他未預期的錯誤
    except Exception as e:
        print(f"其他未預期錯誤: {e}")
        print("請檢查程式邏輯或系統環境")


# ==================== MQTT 客戶端初始化區塊 ====================

# 建立 MQTT 客戶端物件
# 使用 VERSION2 的回調 API 版本，這是較新的 API，避免過時的警告
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# 設定連線時的使用者名稱和密碼（如果有的話）
# 如果 username 和 password 為空字串，表示不使用認證
client.username_pw_set(username, password)

# 將自定義的回調函數指定給客戶端物件
# 當對應的事件發生時，客戶端會自動呼叫這些函數
client.on_connect = on_connect  # 指定連線成功時要執行的函數
client.on_message = on_message  # 指定收到訊息時要執行的函數

# ==================== 建立連線與啟動主循環 ====================

# 連線到 MQTT Broker 伺服器
# 參數說明：
# - broker_address: Broker 伺服器網址
# - port: 通訊埠號
# - keepalive=60: 保持連線的時間（秒），在此時間內若無通訊，會發送心跳封包維持連線
client.connect(broker_address, port, 60)

print("正在嘗試連線到 MQTT Broker...")
print(f"Broker 位址: {broker_address}:{port}")
print(f"訂閱主題: {topic}")
print(f"轉發 API: {BASE_URL}")
print("-" * 50)

# 進入無窮迴圈模式，持續監聽來自 Broker 的訊息
# 這行程式會讓程式持續執行，直到手動停止（Ctrl+C）或發生嚴重錯誤
# 在迴圈中，客戶端會執行以下操作：
# 1. 處理網路流量和訊息接收
# 2. 定期發送心跳封包維持連線
# 3. 自動呼叫相應的回調函數處理各種事件
client.loop_forever()

# 注意事項：
# 1. 程式執行到 client.loop_forever() 會進入無窮循環
# 2. 若要停止程式，請按 Ctrl+C 中斷執行
# 3. 程式停止時會自動清理資源並斷開連線
# 4. 確保網路連線正常，可以同時連接到 MQTT Broker 和 HTTP 伺服器