/*
 * 檔案名稱：commlib.h
 * 程式語言：Arduino C/C++
 * 檔案類型：標頭檔（Header File）- 通用通訊函式庫
 * 
 * ========================================
 * 程式功能完整解說
 * ========================================
 * 
 * 【檔案概述】
 *   本檔案為 Arduino 專案的通用工具函式庫（Common Library），提供一系列
 *   常用的輔助函式，涵蓋字串處理、數值轉換、GPIO 控制、除錯輸出等功能。
 *   這些函式可被多個專案重複使用，提高程式碼的模組化與可維護性。
 * 
 * 【主要功能分類】
 * 
 *   1. 除錯與輸出功能：
 *      - DebugMsg()：除錯訊息輸出（不換行）
 *      - DebugMsgln()：除錯訊息輸出（換行）
 *      - 透過 _Debug 巨集控制是否啟用除錯輸出
 * 
 *   2. GPIO 控制：
 *      - GPIOControl()：控制指定 GPIO 腳位的輸出高低電位
 * 
 *   3. 數學運算：
 *      - POW()：計算數值的次方（整數次方）
 * 
 *   4. 字串生成與格式化：
 *      - SPACE()：生成指定長度的空格字串
 *      - genstr()：生成指定長度的重複字元字串
 *      - strzero()：將數字轉換為指定長度與進位制的字串（補零）
 *      - print2HEX()：將數字轉換為兩位 16 進位字串（不足補零）
 *      - Double2Str()：將浮點數轉換為字串，保留指定小數位數
 * 
 *   5. 數值轉換：
 *      - ULongtoString()：將 unsigned long 轉換為字串
 *      - unstrzero()：將指定進位制的字串轉換為數值
 * 
 *   6. 字串與字元陣列操作：
 *      - chrtoString()：將 char 陣列轉換為 String 物件
 *      - CopyString2Char()：將 String 複製到 char 陣列
 *      - CharCompare()：比較兩個 char 陣列的內容是否相同
 * 
 *   7. JSON 資料處理：
 *      - getjson()：從 HTTP 回應字串中提取 JSON 資料（專為 BMDUINO TCP.h 設計）
 * 
 * 【運作流程說明】
 * 
 *   除錯輸出流程：
 *     1. 檢查全域巨集 _Debug 是否為 1
 *     2. 若為 1，則將訊息輸出至序列埠
 *     3. 若為 0，則不輸出任何訊息（節省資源）
 * 
 *   數值轉字串流程（strzero）：
 *     1. 根據指定的進位制（base）逐位取出數字
 *     2. 將每個位數轉換為對應的字元（0-9, A-F）
 *     3. 儲存於暫存陣列中
 *     4. 反轉順序後組合成最終字串
 * 
 *   JSON 提取流程（getjson）：
 *     1. 搜尋字串中 '[' 與 '{' 的位置
 *     2. 判斷 JSON 格式為陣列（array）或物件（object）
 *     3. 提取從起始符號到對應結束符號的內容
 *     4. 回傳純 JSON 資料字串
 * 
 * 【相依函式庫】
 *   - String.h：Arduino 內建字串處理函式庫
 * 
 * 【全域巨集說明】
 *   - _Debug：除錯模式開關（1：開啟，0：關閉）
 *   - IOon：GPIO 高電位定義（HIGH）
 *   - IOoff：GPIO 低電位定義（LOW）
 * 
 * 【注意事項】
 *   - 除錯輸出功能依賴 Serial 物件，使用前需先執行 Serial.begin()
 *   - strzero() 函式回傳長度固定為 len 參數指定值
 *   - unstrzero() 函式支援 2-16 進位制的字串轉換
 *   - getjson() 函式假設輸入字串中僅包含一組 JSON 資料
 *   - 所有字串操作函式均設計為安全使用，避免緩衝區溢位
 * 
 * 【使用範例】
 *   // 啟用除錯輸出
 *   DebugMsgln("System started");
 *   
 *   // 數字轉換
 *   String hexStr = print2HEX(15);    // 回傳 "0F"
 *   String numStr = strzero(255, 8, 16); // 回傳 "000000FF"
 *   
 *   // GPIO 控制
 *   GPIOControl(13, 1);  // 將 pin 13 設為高電位
 * 
 * 【版本資訊】
 *   最後修改日期：2026年3月26日
 *   適用專案：通用 Arduino 專案
 */

// ========================================
// 全域巨集與函式庫引入
// ========================================
#define _Debug 1        // 除錯模式開啟 (1: 開啟, 0: 關閉)
                        // 當設定為 1 時，DebugMsg 和 DebugMsgln 會輸出訊息
                        // 設定為 0 時，可節省序列埠資源，適合正式發佈版本

#include <String.h>     // 引入 Arduino 內建字串處理函式庫
                        // 提供 String 類別及相關字串操作方法

#define IOon HIGH       // GPIO 高電位定義（通常為 1 或 HIGH）
#define IOoff LOW       // GPIO 低電位定義（通常為 0 或 LOW）

// ========================================
// 函式前置宣告
// ========================================
void DebugMsg(String msg);                    // 除錯訊息輸出函式（不換行）
void DebugMsgln(String msg);                  // 除錯訊息輸出函式（換行）
void GPIOControl(int GP, int cmd);            // GPIO 控制輸出高低電位函式
long POW(long num, int expo);                 // 計算 num 的 expo 次方
String SPACE(int sp);                         // 生成指定長度的空格字串
String ULongtoString(unsigned long ll);       // 將 unsigned long 轉換成字串
String genstr(char c, int sp);                // 生成指定長度的重複字元字串
String strzero(long num, int len, int base);  // 轉換數字為指定長度與進位制的字串，並補零
unsigned long unstrzero(String hexstr, int base); // 轉換指定進位制的字串為數值
String print2HEX(int number);                 // 轉換數字為 16 進位字串，若小於 16 則補 0
String chrtoString(char *p);                  // 將 char 陣列轉為 String 物件
void CopyString2Char(String ss, char *p);     // 複製 String 到 char 陣列
boolean CharCompare(char *p, char *q);        // 比較兩個 char 陣列是否相同
String Double2Str(double dd, int decn);       // 將 double 轉為字串，保留指定小數位數
String getjson(String ss);                    // BMDUINO TCP.h 專用，從 HTTP 回應中提取 JSON

// ========================================
// DebugMsg() 函式：除錯訊息輸出（不換行）
// 參數：msg - 要輸出的訊息字串
// ========================================
void DebugMsg(String msg)
{
    if (_Debug != 0)  // 檢查除錯模式是否開啟（_Debug 不等於 0）
    {
        Serial.print(msg);  // 輸出訊息至序列監控視窗（不換行）
    }
}

// ========================================
// DebugMsgln() 函式：除錯訊息輸出（換行）
// 參數：msg - 要輸出的訊息字串
// ========================================
void DebugMsgln(String msg)
{
    if (_Debug != 0)  // 檢查除錯模式是否開啟（_Debug 不等於 0）
    {
        Serial.println(msg);  // 輸出訊息至序列監控視窗（自動換行）
    }
}

// ========================================
// GPIOControl() 函式：GPIO 控制輸出高低電位
// 參數：
//   GP - GPIO 腳位編號（例如 13）
//   cmd - 控制命令（1：高電位，0：低電位）
// ========================================
void GPIOControl(int GP, int cmd)
{
  // GP：GPIO 腳位號碼
  // cmd：高電位或低電位設定
  //   cmd = 1 時設為高電位（IOon）
  //   cmd = 0 時設為低電位（IOoff）
  
  if (cmd == 1)  // cmd 等於 1，設定為高電位
  {
      digitalWrite(GP, IOon);  // 將指定腳位設為 HIGH
  }
  else if (cmd == 0)  // cmd 等於 0，設定為低電位
  {
      digitalWrite(GP, IOoff); // 將指定腳位設為 LOW
  }
}

// ========================================
// POW() 函式：計算 num 的 expo 次方（整數次方）
// 參數：
//   num - 底數
//   expo - 指數（非負整數）
// 回傳值：num 的 expo 次方結果
// ========================================
long POW(long num, int expo)
{
  long tmp = 1;  // 暫存變數，初始值為 1（任何數的 0 次方為 1）

  if (expo > 0)  // 指數大於零時才進行計算
  { 
    for (int i = 0; i < expo; i++)  // 利用迴圈累乘
    {
      tmp = tmp * num;  // 不斷乘以 num，共執行 expo 次
    }
    return tmp;   // 回傳計算結果
  } 
  else 
  {
    return tmp;   // 若 expo 小於或等於 0，回傳 1
  }
}

// ========================================
// SPACE() 函式：生成指定長度的空格字串
// 參數：sp - 要產生的空格數量
// 回傳值：由指定數量空格組成的字串
// ========================================
String SPACE(int sp)
{
  String tmp = "";  // 建立空字串
  
  for (int i = 0; i < sp; i++)  // 迴圈執行 sp 次
  {
    tmp.concat(' ');  // 每次加入一個空格字元
  }
  
  return tmp;  // 回傳產生的空格字串
}

// ========================================
// ULongtoString() 函式：將 unsigned long 轉換成字串
// 參數：ll - 要轉換的無號長整數（0 ~ 4,294,967,295）
// 回傳值：轉換後的字串
// 
// 重要概念解釋：
//   unsigned long ll：32 位元的無號整數，常用於儲存時間（如 millis()）
//   或 RFID 的 UID 等較大數值
//   char ww[20]：C 語言的傳統字串緩衝區（字元陣列）
//   sprintf(ww, "%lu", ll)：將數值格式化為十進位字串並存入陣列
//   %lu：格式符號，表示將 unsigned long 以十進位輸出
// ========================================
String ULongtoString(unsigned long ll)
{
  // 宣告一個字元陣列 ww，大小為 20，用來暫存格式化後的字串
  // 20 個字元足夠容納 unsigned long 的最大值（10 位數）加上結尾字元
  char ww[20];

  // 使用 sprintf 函式將 unsigned long 數值格式化為十進位字串
  // %lu 代表「以 unsigned long 格式印出為十進位」
  sprintf(ww, "%lu", ll);

  // 將 C-style 字串轉換為 Arduino String 物件並回傳
  return String(ww);
}

// ========================================
// genstr() 函式：生成指定長度的重複字元字串
// 參數：
//   c - 要重複的字元
//   sp - 重複次數（字串長度）
// 回傳值：由指定字元重複組成的字串
// ========================================
String genstr(char c, int sp)
{
  String tmp = "";  // 建立空字串
  
  for (int i = 0; i < sp; i++)  // 迴圈執行 sp 次
  {
    tmp.concat(c);  // 每次加入指定的字元
  }
  
  return tmp;  // 回傳產生的字串
}

// ========================================
// strzero() 函式：轉換數字為指定長度與進位制的字串，並補零
// 參數：
//   num - 要轉換的數字
//   len - 目標字串長度
//   base - 進位制（2 進位、10 進位、16 進位等）
// 回傳值：格式化後的字串（長度固定為 len，不足補零）
// 
// 演算法說明：
//   1. 使用除法取餘數的方式，逐位取出數字在各進位制下的表示
//   2. 將餘數轉換為對應的字元（0-9 或 A-F）
//   3. 將字元存入暫存陣列（從低位到高位）
//   4. 最後反轉陣列順序，組合成正確順序的字串
// ========================================
String strzero(long num, int len, int base)
{
  // num：傳入的數字
  // len：目標回傳字串長度
  // base：進位制（例如 2、10、16）
  
  String retstring = String("");  // 建立空白字串，用於儲存結果
  int ln = 1;          // 迴圈計數器，從 1 開始
  int i = 0;           // 迴圈索引變數
  char tmp[10];        // 暫存字元陣列，儲存轉換過程中的字元
  long tmpnum = num;   // 目前剩餘的數字
  int tmpchr = 0;      // 暫存當前位數的數值
  
  // 16 進位對照表：將 0-15 對應到 '0'-'9' 和 'A'-'F'
  char hexcode[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  
  // 逐位取出數字（從最低位開始）
  while (ln <= len)  // 繼續直到取出足夠的位數
  {
    tmpchr = (int)(tmpnum % base);      // 取得當前最低位的數值（餘數）
    tmp[ln - 1] = hexcode[tmpchr];      // 將數值轉換為對應的字元並存入陣列
    ln++;                               // 計數器加 1
    tmpnum = (long)(tmpnum / base);     // 除以進位制，準備取下一位數
  }
  
  // 反轉陣列順序，將低位在前的順序轉為高位在前
  for (i = len - 1; i >= 0; i--)
  {
    retstring.concat(tmp[i]);  // 從陣列最後一個元素開始連接
  }
  
  return retstring;  // 回傳格式化後的字串
}

// ========================================
// unstrzero() 函式：轉換指定進位制的字串為數值
// 參數：
//   hexstr - 要轉換的字串（例如 "FF"）
//   base - 進位制（例如 16）
// 回傳值：轉換後的無號長整數
// 
// 演算法說明：
//   1. 將輸入字串轉為大寫
//   2. 逐字元取出，查詢對應的數值（0-15）
//   3. 根據位數權重累加計算總值
// ========================================
unsigned long unstrzero(String hexstr, int base) 
{
  String chkstring;      // 暫存字串（本函式中未實際使用）
  int len = hexstr.length();     // 取得輸入字串的長度
  unsigned int i = 0;            // 迴圈索引
  unsigned int tmp = 0;          // 暫存當前字元的 ASCII 碼
  unsigned int tmp1 = 0;         // 暫存當前字元對應的數值
  unsigned long tmpnum = 0;      // 累加計算的最終數值
  
  // 16 進位對照表（同時適用於 2-16 進位）
  String hexcode = String("0123456789ABCDEF");
  
  for (i = 0; i < len; i++)
  {
    hexstr.toUpperCase();                // 將整個字串轉為大寫，統一格式
    tmp = hexstr.charAt(i);              // 取出第 i 個字元
    tmp1 = hexcode.indexOf(tmp);         // 在對照表中尋找該字元的位置（即數值）
    
    // 計算該位數的貢獻值
    // POW(base, (len - i - 1))：計算該位數的權重（base 的冪次）
    // 例如：16 進位的第 1 位（最左）權重為 base^(len-1)
    tmpnum = tmpnum + tmp1 * POW(base, (len - i - 1));
  }
  
  return tmpnum;  // 回傳轉換後的數值
}

// ========================================
// print2HEX() 函式：轉換數字為 16 進位字串，若小於 16 則補 0
// 參數：number - 要轉換的數字（0-255）
// 回傳值：兩位數的 16 進位字串（例如 5 回傳 "05"）
// ========================================
String print2HEX(int number) 
{
  String ttt;  // 暫存字串
  
  if (number >= 0 && number < 16)  // 判斷數字是否小於 16
  {
    // 若小於 16，補一個 '0' 在前方，確保回傳兩位數
    ttt = String("0") + String(number, HEX);
  }
  else
  {
    // 大於等於 16，直接轉換為 16 進位字串
    ttt = String(number, HEX);
  }
  
  return ttt;  // 回傳結果
}

// ========================================
// chrtoString() 函式：將 char 陣列轉為 String 物件
// 參數：p - 指向 char 陣列的指標
// 回傳值：轉換後的 String 物件
// ========================================
String chrtoString(char *p)
{
    String tmp;      // 暫存字串
    char c;          // 暫存當前字元
    int count = 0;   // 計數器
    
    while (count < 100)  // 最多處理 100 個字元（避免無限迴圈）
    {
        c = *(p + count);  // 取得第 count 個字元
        if (c != 0x00)     // 若非結束字元（'\0'）
        {
            tmp.concat(String(c));  // 將字元加入字串
        }
        else
        {
            return tmp;  // 遇到結束字元，回傳結果
        }
        count++;  // 計數器加 1
    }
    
    return tmp;  // 回傳結果（理論上不會執行到此）
}

// ========================================
// CopyString2Char() 函式：複製 String 到 char 陣列
// 參數：
//   ss - 來源字串（String 物件）
//   p - 目標 char 陣列的指標
// ========================================
void CopyString2Char(String ss, char *p)
{
  if (ss.length() <= 0)  // 檢查是否為空字串
  {
    *p = 0x00;  // 加上字串結束字元 '\0'
    return;     // 結束函式
  }
  
  // 使用 toCharArray() 將 String 複製到 char 陣列
  // 第二個參數為複製長度（需包含結束字元）
  ss.toCharArray(p, ss.length() + 1);
}

// ========================================
// CharCompare() 函式：比較兩個 char 陣列是否相同
// 參數：
//   p - 第一個 char 陣列的指標
//   q - 第二個 char 陣列的指標
// 回傳值：true（相同）或 false（不相同）
// ========================================
boolean CharCompare(char *p, char *q) 
{
  boolean flag = false;   // 是否結束旗標（本函式中未實際使用）
  int count = 0;          // 計數器
  int nomatch = 0;        // 不相同計數器
  
  while (count < 100)     // 最多比較 100 個字元
  {
      // 檢查是否遇到任一字串的結束字元
      if (*(p + count) == 0x00 || *(q + count) == 0x00)
          break;  // 結束比較迴圈
      
      // 比較當前位置的字元是否相同
      if (*(p + count) != *(q + count))
      {
          nomatch++;  // 若不同，不相同計數器加 1
      }
      count++;  // 計數器加 1
  }
  
  return nomatch == 0;  // 若不相同計數器為 0，表示完全相同，回傳 true
}

// ========================================
// Double2Str() 函式：將 double 轉為字串，保留指定小數位數
// 參數：
//   dd - 要轉換的浮點數
//   decn - 要保留的小數位數
// 回傳值：格式化後的字串（例如 Double2Str(12.3456, 2) 回傳 "12.34"）
// ========================================
String Double2Str(double dd, int decn)
{
  int a1 = (int)dd;   // 取得整數部分（直接轉型會捨去小數）
  int a3;             // 儲存小數部分放大後的整數值
  
  if (decn > 0)  // 若需要保留小數位數
  {
      double a2 = dd - a1;              // 取得小數部分
      a3 = (int)(a2 * pow(10, decn));   // 將小數部分放大 decn 倍並轉為整數
  }
  
  if (decn > 0)  // 需要保留小數位數
  {
      // 組合字串：整數 + 小數點 + 小數部分
      return String(a1) + "." + String(a3);
  }
  else
  {
      // 不需要小數部分，只回傳整數
      return String(a1);
  }
}

// ========================================
// getjson() 函式：從 HTTP 回應字串中提取 JSON 資料
// 專為 BMDUINO TCP.h 設計，用於處理 Wifi.http_getString() 的回傳結果
// 參數：ss - 包含 JSON 資料的原始 HTTP 回應字串
// 回傳值：純 JSON 資料字串（去除 HTTP 標頭等額外內容）
// 
// 處理邏輯：
//   1. 搜尋 '[' 和 '{' 在字串中的位置
//   2. 根據位置判斷 JSON 格式為陣列（array）或物件（object）
//   3. 找到對應的結束符號（']' 或 '}'）
//   4. 提取從起始到結束的內容
// ========================================
String getjson(String ss)
{
  String tmp = "";           // 暫存結果字串
  int s1 = ss.indexOf('[');  // 尋找陣列起始符號 '[' 的位置
  int s2 = ss.indexOf('{');  // 尋找物件起始符號 '{' 的位置
  int st1, st2;              // 起始和結束位置
  
  if (s1 == -1)  // 沒有找到 '['（不是陣列格式）
  {
    if (s2 == -1)  // 也沒有找到 '{'
    {
      return tmp;  // 沒有 JSON 資料，回傳空字串
    }
    else  // 找到 '{'，表示為 JSON 物件
    {
      st1 = s2;                     // 起始位置為 '{' 的位置
      st2 = ss.lastIndexOf("}");    // 尋找最後一個 '}' 的位置
      tmp = ss.substring(st1, st2 + 1);  // 提取 JSON 物件內容
      return tmp;
    }
  }
  else  // 找到 '['（可能為陣列格式）
  {
    if (s1 < s2)  // '[' 在 '{' 之前，表示為 JSON 陣列
    {
      st1 = s1;                     // 起始位置為 '[' 的位置
      st2 = ss.lastIndexOf("]");    // 尋找最後一個 ']' 的位置
      tmp = ss.substring(st1, st2 + 1);  // 提取 JSON 陣列內容
      return tmp;
    }
    else  // '{' 在 '[' 之前，表示為 JSON 物件
    {
      st1 = s2;                     // 起始位置為 '{' 的位置
      st2 = ss.lastIndexOf("}");    // 尋找最後一個 '}' 的位置
      tmp = ss.substring(st1, st2 + 1);  // 提取 JSON 物件內容
      return tmp;
    }
  }
}