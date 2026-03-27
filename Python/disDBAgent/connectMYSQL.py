"""
MySQL 資料庫連線測試程式
功能：測試與 MySQL 資料庫的連線，並查詢資料庫版本資訊
作者：自動生成繁體中文註解版本
日期：2024-11-25
"""

# ==================== 導入必要的套件 ====================

# 匯入 pymysql 套件，並命名為 DB
# pymysql 是一個純 Python 實現的 MySQL 客戶端庫，用於連接和操作 MySQL 資料庫
import pymysql as DB

# ==================== 資料庫連線設定 ====================

# 建立資料庫連線物件
# DB.connect() 方法用於建立與 MySQL 資料庫的連接，參數說明如下：
db = DB.connect(
    host='localhost',  # 資料庫伺服器地址
    # 'localhost' 表示本機，如果是遠端資料庫則填寫 IP 位址或網域名稱
    # 例如：'192.168.1.100' 或 'mysql.example.com'

    port=3306,  # 資料庫伺服器通訊埠
    # MySQL 預設使用 3306 埠，如有變更請修改為對應的埠號
    # 常見的資料庫埠號：
    # MySQL: 3306, PostgreSQL: 5432, SQL Server: 1433, Oracle: 1521

    user='big',  # 資料庫使用者名稱
    # 用於登入資料庫的帳號名稱
    # 必須具有足夠的權限來執行後續的查詢操作

    passwd='12345678',  # 資料庫使用者密碼
    # 對應使用者名稱的密碼
    # 注意：在實際應用中，建議不要將密碼硬編碼在程式碼中
    # 可考慮使用環境變數、設定檔或密碼管理器來管理敏感資訊

    db='big',  # 要連接的資料庫名稱
    # 指定要操作的資料庫名稱
    # 如果未指定，則連線到預設資料庫，後續可使用 USE database_name 切換

    charset='utf8'  # 資料庫編碼格式
    # 設定連線使用的字符集，確保中文等 Unicode 字元正確顯示
    # 常用編碼：
    # 'utf8': UTF-8 編碼（MySQL 中的 utf8 最多支援 3 位元組）
    # 'utf8mb4': 完整的 UTF-8 編碼（支援 4 位元組，如表情符號）
    # 'big5': 繁體中文編碼（台灣常用）
    # 'gbk': 簡體中文編碼
)

# ==================== 建立資料庫游標 ====================

# 建立操作游標（cursor）
# 游標是資料庫操作的核心物件，主要功能包括：
cursor = db.cursor()
# 1. 執行 SQL 語句（查詢、插入、更新、刪除等）
# 2. 獲取查詢結果
# 3. 控制事務（提交、回滾）
#
# 游標類型說明：
# cursor = db.cursor()                    # 預設游標，返回元組（tuple）
# cursor = db.cursor(pymysql.cursors.DictCursor)  # 字典游標，返回字典
# cursor = db.cursor(pymysql.cursors.SSCursor)    # 無緩衝游標，適用大量數據

# ==================== 準備 SQL 查詢語句 ====================

# 定義 SQL 查詢語句
# 使用 SELECT VERSION() 查詢 MySQL 資料庫的版本資訊
sql = 'SELECT VERSION()'
#
# 其他常用的測試查詢：
# 1. 查詢當前時間：SELECT NOW()
# 2. 查詢當前使用者：SELECT USER()
# 3. 查詢資料庫列表：SHOW DATABASES
# 4. 查詢資料表列表：SHOW TABLES

# ==================== 執行 SQL 查詢 ====================

# 使用游標執行 SQL 語句
cursor.execute(sql)
#
# execute() 方法說明：
# - 執行單條 SQL 語句
# - 返回受影響的行數（對於 SELECT 查詢，返回檢索到的行數）
#
# 注意事項：
# 1. SQL 語句不需要以分號結尾（游標會自動處理）
# 2. 如果要執行多條語句，需使用 executemany() 方法
# 3. 建議使用參數化查詢來防止 SQL 注入攻擊：
#    sql = "SELECT * FROM users WHERE username = %s"
#    cursor.execute(sql, ('admin',))

# ==================== 獲取查詢結果 ====================

# 選取第一筆結果
data = cursor.fetchone()
#
# 常用的結果獲取方法：
# 1. fetchone(): 獲取下一行資料（單筆記錄）
# 2. fetchall(): 獲取所有剩餘的資料（全部記錄）
# 3. fetchmany(size): 獲取指定數量的資料
# 4. rowcount: 取得受影響的行數
#
# 資料格式：
# - 預設游標：返回元組，如：('8.0.33',)
# - 字典游標：返回字典，如：{'VERSION()': '8.0.33'}

# ==================== 顯示查詢結果 ====================

# 打印資料庫版本資訊
print("Database version : %s " % data)
#
# 輸出範例：
# Database version : ('8.0.33',)
#
# 如果要取得版本字串（去除元組括號）：
# print("Database version : %s " % data[0])
# 輸出：Database version : 8.0.33
#
# 或者使用格式化字串（f-string）：
# print(f"Database version : {data[0]}")
#
# 完整版本資訊解析：
# MySQL 版本號格式：主版本號.次版本號.修訂號
# 例如：8.0.33 表示主版本 8、次版本 0、修訂號 33

# ==================== 關閉資料庫連線 ====================

# 關閉資料庫連線
db.close()
#
# 關連線的重要性：
# 1. 釋放資料庫連接資源
# 2. 避免連接池耗盡
# 3. 確保資料完整性
#
# 最佳實踐：
# 1. 總是關閉不再使用的連接
# 2. 使用 with 語句自動管理資源
# 3. 考慮使用連接池管理大量連接
#
# 替代方案（使用 with 語句自動管理）：
# with DB.connect(...) as db:
#     cursor = db.cursor()
#     cursor.execute('SELECT VERSION()')
#     data = cursor.fetchone()
#     print(f"Database version: {data[0]}")
# # 離開 with 區塊後，連接會自動關閉

# ==================== 完整錯誤處理版本建議 ====================

"""
以下是加上完整錯誤處理的建議版本：

import pymysql as DB

try:
    # 建立資料庫連線
    db = DB.connect(
        host='localhost',
        port=3306,
        user='big',
        passwd='12345678',
        db='big',
        charset='utf8'
    )

    print("資料庫連線成功！")

    # 建立游標
    cursor = db.cursor()

    # 執行查詢
    cursor.execute('SELECT VERSION()')

    # 取得結果
    data = cursor.fetchone()

    # 顯示結果
    print(f"MySQL 資料庫版本: {data[0]}")

    # 可選：查詢更多資訊
    cursor.execute('SELECT NOW()')
    current_time = cursor.fetchone()
    print(f"當前資料庫時間: {current_time[0]}")

    cursor.execute('SELECT DATABASE()')
    db_name = cursor.fetchone()
    print(f"當前資料庫: {db_name[0]}")

except DB.Error as e:
    # 處理資料庫相關錯誤
    print(f"資料庫錯誤發生: {e}")

except Exception as e:
    # 處理其他錯誤
    print(f"程式執行錯誤: {e}")

finally:
    # 確保資源被正確釋放
    try:
        if 'cursor' in locals():
            cursor.close()
        if 'db' in locals() and db.open:
            db.close()
            print("資料庫連線已關閉")
    except Exception as e:
        print(f"關閉連線時發生錯誤: {e}")
"""

print("程式執行完成")