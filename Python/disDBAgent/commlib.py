"""
通用工具函式庫 (commlib.py)
功能：提供系統資訊取得、時間處理、網路工具等常用功能
作者：自動生成繁體中文註解版本
日期：2024-11-25
"""

# ==================== 導入必要的套件 ====================

from datetime import datetime  # 日期時間處理模組，用於取得系統時間
import string  # 字串處理模組，提供字串相關常數和函式
import socket  # 網路通訊模組，用於取得本機 IP 地址
import uuid  # 通用唯一識別碼模組，用於取得 MAC 地址


# ==================== 通用工具函式 ====================

def strzero(no, ll):
    """
    數字補零函式
    將數字轉換為字串，並在前面補零到指定長度

    參數：
    no (int/str): 要處理的數字，可以是整數或字串
    ll (int): 目標總長度（補零後的總字元數）

    返回值：
    str: 補零後的數字字串

    範例：
    >>> strzero(5, 3)
    '005'
    >>> strzero(123, 2)
    '123'  # 因為原始長度已超過目標長度，保持不變
    >>> strzero('7', 4)
    '0007'
    """
    # 將輸入轉換為字串（無論輸入是數字或字串）
    tmp = str(no)

    # 檢查字串長度是否已達到或超過目標長度
    if len(tmp) >= ll:
        # 已達到目標長度，直接返回原字串
        return tmp
    else:
        # 長度不足，使用 rjust() 方法在左側補零
        # rjust(width, fillchar) 將字串靠右對齊，並用指定字元填充左側
        # 計算需要補零的數量：目標長度 - 當前長度 + 1（確保足夠補齊）
        return tmp.rjust(ll - len(tmp) + 1, '0')


# 注意：這裡有兩個同名的 strzero 函式定義
# 在 Python 中，後定義的函式會覆蓋先定義的函式
# 這可能是程式碼錯誤或維護中的重複，建議只保留一個定義

def getsystime():
    """
    取得系統時間字串函式
    取得當前系統時間，並格式化為指定格式的數字字串

    返回值：
    str: 14位數字的時間字串，格式為 YYYYMMDDHHMMSS
          例如：20231125143022 表示 2023年11月25日 14時30分22秒

    格式說明：
    YYYY: 4位數年份（如 2023）
    MM:   2位數月份（01-12）
    DD:   2位數日期（01-31）
    HH:   2位數小時（00-23，24小時制）
    MM:   2位數分鐘（00-59）
    SS:   2位數秒鐘（00-59）

    使用範例：
    >>> getsystime()
    '20231125143022'

    應用場景：
    1. 資料庫時間戳記
    2. 檔案命名（避免重複）
    3. 日誌記錄時間
    4. 系統事件標記
    """
    # 取得當前系統時間
    now = datetime.now()

    # 將各時間元件轉換為字串並補零，然後串接成完整時間字串
    return (strzero(now.year, 4) +  # 年份補零到4位
            strzero(now.month, 2) +  # 月份補零到2位
            strzero(now.day, 2) +  # 日期補零到2位
            strzero(now.hour, 2) +  # 小時補零到2位
            strzero(now.minute, 2) +  # 分鐘補零到2位
            strzero(now.second, 2))  # 秒鐘補零到2位


def get_local_ip():
    """
    取得本機 IP 地址函式
    透過建立虛擬網路連線的方式取得本機的 IP 地址

    返回值：
    str: 本機 IP 地址字串（IPv4）
          例如：'192.168.1.100' 或 '10.0.0.1'
          如果取得失敗，返回迴環地址 '127.0.0.1'

    工作原理：
    1. 建立一個 UDP socket（不需要實際連線）
    2. 嘗試連線到一個廣播地址（10.255.255.255）
    3. 取得 socket 的本機地址
    4. 關閉 socket 並返回 IP

    注意事項：
    - 10.255.255.255 是一個特殊的廣播地址，用於取得網路介面資訊
    - 此方法在大多數情況下都能正確取得本機 IP
    - 如果有多個網路介面，會返回預設路由的 IP

    使用範例：
    >>> get_local_ip()
    '192.168.1.100'

    替代方案：
    1. 使用主機名稱：socket.gethostbyname(socket.gethostname())
    2. 枚舉所有網路介面：使用 netifaces 套件
    """
    try:
        # 建立一個 UDP socket 物件
        # AF_INET: 使用 IPv4 協定
        # SOCK_DGRAM: 使用 UDP 協定（不需要建立實際連線）
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # 利用 UDP 協定的特性，嘗試連線到一個廣播地址
        # 這個連線實際上不會傳送數據，只是為了取得本機 IP
        # '10.255.255.255': 一個保留的廣播地址
        # 1: 任意埠號（不重要）
        s.connect(('10.255.255.255', 1))

        # 取得 socket 的本機地址
        # getsockname() 返回 (ip_address, port) 元組
        # [0] 取得 IP 地址部分
        local_ip = s.getsockname()[0]

    except Exception as e:
        # 如果發生任何錯誤，返回迴環地址（localhost）
        # 迴環地址總是可以使用的，但只能在本機存取
        local_ip = '127.0.0.1'
        # 可選：記錄錯誤資訊（實際應用中可以加入日誌記錄）
        # print(f"取得本機 IP 失敗: {e}")

    finally:
        # 無論成功或失敗，都要關閉 socket 釋放資源
        s.close()

    return local_ip


def get_mac_address():
    """
    取得 MAC 地址函式（無分隔符版本）
    取得本機網路卡的 MAC 地址，返回連續的12位十六進制字串

    返回值：
    str: 12位大寫十六進制 MAC 地址（無分隔符）
          例如：'001122AABBCC'

    工作原理：
    1. 使用 uuid.getnode() 取得網路卡的硬體地址（整數格式）
    2. 轉換為十六進制字串
    3. 取出最後12位（MAC 地址是48位，十六進制表示為12位）
    4. 格式化成連續字串

    注意事項：
    - 此方法取得的是第一個網路介面的 MAC 地址
    - MAC 地址在系統中是唯一的
    - 虛擬機器或容器可能有不同的 MAC 地址

    使用範例：
    >>> get_mac_address()
    '001122AABBCC'

    應用場景：
    1. 裝置唯一識別碼
    2. 軟體授權驗證
    3. 網路設備管理
    4. 安全驗證
    """
    # 取得網路卡節點標識符（通常就是 MAC 地址）
    # uuid.getnode() 返回整數格式的硬體地址
    node_id = uuid.getnode()  # 例如：54863803075678

    # 將整數轉換為 UUID 物件，然後取得十六進制表示
    # uuid.UUID(int=node_id): 從整數建立 UUID 物件
    # .hex: 取得 UUID 的十六進制字串表示（32位）
    # [-12:]: 取最後12位（MAC 地址部分）
    mac = uuid.UUID(int=node_id).hex[-12:]  # 例如：'001122aabbcc'

    # 將 MAC 地址轉換為大寫，並使用列表推導式每2位一組
    # range(0, 11, 2): 產生序列 [0, 2, 4, 6, 8, 10]
    # mac[e:e+2]: 取出每2個字元
    # "".join(): 將所有部分連接成一個字串
    return "".join([mac[e:e + 2] for e in range(0, 11, 2)]).upper()


def get_mac_address2():
    """
    取得 MAC 地址函式（冒號分隔版本）
    取得本機網路卡的 MAC 地址，返回標準格式的 MAC 地址字串

    返回值：
    str: 標準格式的 MAC 地址，每2位用冒號分隔
          例如：'00:11:22:AA:BB:CC'

    格式說明：
    MAC 地址標準格式是 6 組十六進制數字，每組2位，用冒號分隔
    總共17個字元（12位數字 + 5個分隔符）

    與 get_mac_address() 的區別：
    - 此函式返回帶冒號分隔的標準格式
    - get_mac_address() 返回連續不帶分隔符的格式

    使用範例：
    >>> get_mac_address2()
    '00:11:22:AA:BB:CC'

    應用場景：
    1. 網路設定顯示
    2. 系統管理工具
    3. 網路診斷
    4. 設備清單
    """
    # 取得網路卡節點標識符
    node_id = uuid.getnode()

    # 轉換為十六進制字串並取最後12位
    mac = uuid.UUID(int=node_id).hex[-12:]

    # 將 MAC 地址每2位一組，用冒號分隔
    # ":".join(): 用冒號連接所有部分
    return ":".join([mac[e:e + 2] for e in range(0, 11, 2)]).upper()
def get_current_datetime(format_str="%Y-%m-%d %H:%M:%S"):
    # 取得當前時間並格式化為指定格式
    return datetime.now().strftime(format_str)

def get_system_info():
    # 取得系統綜合資訊
    import platform
    return {
        'system': platform.system(),
        'release': platform.release(),
        'version': platform.version(),
        'machine': platform.machine(),
        'processor': platform.processor(),
        'hostname': socket.gethostname(),
        'local_ip': get_local_ip(),
        'mac_address': get_mac_address2(),
        'timestamp': getsystime()
    }

def is_valid_ip(ip_address):
    # 驗證 IP 地址格式是否正確
    try:
        socket.inet_aton(ip_address)
        return True
    except socket.error:
        return False

def generate_random_string(length=8):
    # 產生隨機字串
    import random
    import string
    chars = string.ascii_letters + string.digits
    return ''.join(random.choice(chars) for _ in range(length))