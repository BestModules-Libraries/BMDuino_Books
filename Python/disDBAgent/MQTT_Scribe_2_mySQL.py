"""
MQTT 到 MySQL 資料庫轉存程式
功能：訂閱 MQTT 主題，接收感測器數據，儲存到 MySQL 資料庫
作者：自動生成繁體中文註解版本
日期：2024-11-25
"""

# ==================== 導入必要的套件 ====================

# 匯入 MQTT 客戶端套件，並命名為 mqtt，用於 MQTT 通訊協定
import paho.mqtt.client as mqtt

# 匯入 json 套件，用來處理 JSON 格式資料的編碼與解碼
import json

# 匯入自定義的通用工具模組 commlib，內含例如 get_local_ip()、getsystime() 等函式
# commlib 模組可能包含以下功能：
# - get_local_ip(): 取得本地電腦的 IP 地址
# - getsystime(): 取得系統目前時間，格式可能為 20220802143022（年月日時分秒）
from commlib import *

# ------------------------------ # 

# 匯入 pymysql 套件，並命名為 DB，作為資料庫連線與操作的工具
import pymysql as DB

# ==================== 資料庫連線設定 ====================

# 建立資料庫連線物件，參數說明如下：
# host：資料庫主機名稱（localhost 表示本機）
# port：連接埠號（MySQL 預設為 3306）
# user：登入資料庫使用者名稱
# passwd：登入密碼
# db：要連接的資料庫名稱
# charset：資料編碼格式，使用 utf8，確保中文等字元正確儲存
db = DB.connect(
    host='localhost',  # 資料庫主機位址
    port=3306,  # MySQL 預設通訊埠
    user='big',  # 資料庫使用者名稱
    passwd='12345678',  # 資料庫使用者密碼
    db='big',  # 要連接的資料庫名稱
    charset='utf8'  # 使用 UTF-8 編碼
)

# 建立資料庫游標，用來執行 SQL 指令
# 游標（cursor）是資料庫操作的核心物件，用於執行查詢和獲取結果
cursor = db.cursor()

# 測試用的 SQL 指令，插入一筆固定的資料到 dhtdata 資料表
# 這行程式碼主要用於測試資料庫連線是否正常
sql1 = "INSERT INTO dhtdata (id, MAC, crtdatetime, temperature, humidity, systime) VALUES (NULL, 'a01110001', current_timestamp(), '25.3', '88.9', '20220624220101');"

# ------------------------------ # 

# ==================== MQTT 連線設定 ====================

# 設定 MQTT Broker 的連線資訊
broker_address = "broker.emqx.io"  # MQTT Broker 的位址，使用公開的 EMQX 測試伺服器
port = 1883  # 使用非加密的 MQTT 預設埠號
username = ""  # 使用者名稱（留空表示匿名登入）
password = ""  # 密碼（留空表示匿名登入）
topic = "/arduino/dht/#"  # 要訂閱的主題（topic），# 是萬用字元

# 預期從 MQTT 接收到的資料格式（JSON）：
# {
#   "Device": "E89F6DE8F3BC",  # 裝置的 MAC 地址或唯一識別碼
#   "Temperature": 24,          # 溫度值（攝氏度）
#   "Humidity": 77              # 濕度值（百分比）
# }

# ==================== SQL 語法模板 ====================

# SQL 字串模板，將接收到的 JSON 解析後組合為 SQL 插入語法
# 欄位說明：
# - MAC: 裝置的 MAC 地址（從 JSON 的 Device 欄位取得）
# - IP: 本地電腦的 IP 地址（從 get_local_ip() 函數取得）
# - temperature: 溫度值（從 JSON 的 Temperature 欄位取得）
# - humidity: 濕度值（從 JSON 的 Humidity 欄位取得）
# - systime: 系統時間（從 getsystime() 函數取得）
sqlstr0 = "INSERT INTO dhtdata (MAC, IP, temperature, humidity, systime) VALUES ('%s', '%s', %f, %f, '%s');"


# ==================== MQTT 回調函數定義 ====================

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
    # 顯示訊息來源的主題和原始數據
    print("接收到來自主題的數據: " + str(msg.payload))
    print("Data coming from " + msg.topic)  # 顯示主題名稱

    # ==================== 步驟 1：解碼位元組數據 ====================
    # 將接收到的位元組資料解碼為 UTF-8 格式的字串
    # decode("utf-8") 將 bytes 類型轉換為 string 類型
    Payload = msg.payload.decode("utf-8")

    # ==================== 步驟 2：解析 JSON 數據 ====================
    # 將 JSON 格式的字串解析為 Python 字典物件
    # json.loads() 函數將 JSON 字串轉換為 Python 字典
    jsondata = json.loads(Payload)

    # ==================== 步驟 3：格式化並顯示 JSON 數據 ====================
    # 將字典轉換為格式化的 JSON 字串（方便閱讀）
    # json.dumps() 函數將 Python 字典轉換為格式化的 JSON 字串
    # - ensure_ascii=False: 允許非 ASCII 字元（如中文）正確顯示
    # - indent=4: 使用 4 個空格縮排，讓 JSON 更易讀
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

    # ==================== 步驟 5：組合 SQL 插入語句 ====================
    # 根據接收到的 JSON 數據組合 SQL 語法
    # 使用 sqlstr0 模板，插入以下資料：
    # 1. device: 裝置 MAC 地址
    # 2. get_local_ip(): 從 commlib 模組取得本地 IP 地址
    # 3. temperature: 溫度值（浮點數）
    # 4. humidity: 濕度值（浮點數）
    # 5. getsystime(): 從 commlib 模組取得系統時間
    sqlstr = sqlstr0 % (
        device,  # %s: 字串類型的裝置 MAC
        get_local_ip(),  # %s: 字串類型的本地 IP
        temperature,  # %f: 浮點數類型的溫度值
        humidity,  # %f: 浮點數類型的濕度值
        getsystime()  # %s: 字串類型的系統時間
    )

    # 顯示組合完成的 SQL 指令（用於除錯）
    print("即將執行的 SQL 指令:")
    print(sqlstr)

    # ==================== 步驟 6：執行 SQL 指令 ====================
    # 使用資料庫游標執行 SQL 指令，將資料插入資料庫中
    # cursor.execute() 執行 SQL 語句
    cursor.execute(sqlstr)

    # 注意：這裡沒有使用 db.commit()，可能需要根據需求手動提交事務
    # 如果沒有自動提交，需要加上 db.commit() 才能將數據真正寫入資料庫
    # db.commit()  # 如果需要手動提交，請取消註解這行


# ------------------------------ #

# ==================== MQTT 客戶端初始化 ====================

# 建立 MQTT 客戶端物件
# 建立一個 MQTT 客戶端實例，用於連接到 MQTT Broker
client = mqtt.Client()

# 設定使用者帳號與密碼（如需登入）
# 如果 username 和 password 為空字串，表示不使用認證
client.username_pw_set(username, password)

# 設定連線與訊息處理的回調函數
# 當對應的事件發生時，客戶端會自動呼叫這些函數
client.on_connect = on_connect  # 設定當成功連線後要執行的函式
client.on_message = on_message  # 設定當收到訊息時要執行的函式

# 與 MQTT Broker 建立連線
# 參數說明：
# - broker_address: MQTT Broker 位址
# - port: 通訊埠號
# - keepalive=60: 保持連線的時間（秒）
#   若超過 60 秒未傳送訊息，Broker 會發送 PING 請求以保持連線存活
client.connect(broker_address, port, 60)

# ==================== 啟動 MQTT 事件循環 ====================

# 進入 MQTT 事件循環，持續監聽與處理訂閱的主題訊息
# client.loop_forever() 會讓程式進入無窮迴圈，持續：
# 1. 監聽 MQTT 訊息
# 2. 處理接收到的訊息
# 3. 維持與 Broker 的連線
print("MQTT 客戶端已啟動，開始監聽主題: " + topic)
print("正在等待接收感測器數據...")
print("-" * 50)

client.loop_forever()

# 注意事項：
# 1. 程式執行到 client.loop_forever() 會進入無窮循環
# 2. 若要停止程式，請按 Ctrl+C 中斷執行
# 3. 建議在程式中加入錯誤處理和資料庫事務管理
# 4. 確保 MySQL 服務正在運行且連線參數正確
# 5. commlib 模組必須存在於 Python 路徑中