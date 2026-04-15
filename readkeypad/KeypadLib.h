/*
BMK52T016 模組：由 BestModules 推出的 I2C 介面按鍵矩陣模組，支援中斷觸發偵測按鍵事件。

getINT()：檢查模組的中斷腳是否為 LOW，若為 LOW 表示有按鍵被按下。

readKeyValue()：讀取當前按下的按鍵所對應的 16-bit 數值（每個 bit 代表一顆按鍵）。

BIN：將數值以二進位方式印出，方便觀察每個按鍵（bit）是否被觸發。
keymask[] 與 readKey() 是整個模組的核心，透過 bitmask 技巧可以快速檢查是哪一顆鍵被按下。

buttondelay[] 通常可用來設定每個按鍵的觸發靈敏度或去彈跳時間。

回傳 'E' 在 inputKey() 中是一個錯誤保護，防止程式回傳空字元。

*/

#include "BMK52T016.h"  // 引入 BMK52T016 模組的函式庫

// 定義每個按鍵的延遲時間（通常用於按鍵去彈跳）
uint8_t buttondelay[] = {
  2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2
};

// 定義每個按鍵對應的 bit 掩碼（keymask）
// 用 16-bit 二進位表示，從右到左代表 KEY1 ~ KEY16
uint16_t keymask[] = {
  0b0000000000000001, 0b0000000000000010, 0b0000000000000100, 0b0000000000001000,
  0b0000000000010000, 0b0000000000100000, 0b0000000001000000, 0b0000000010000000,
  0b0000000100000000, 0b0000001000000000, 0b0000010000000000, 0b0000100000000000,
  0b0001000000000000, 0b0010000000000000, 0b0100000000000000, 0b1000000000000000
};

// 定義每顆按鍵對應的字元，用於回傳指令，例如 '1', '2', 'A', '*', '#', 等
char keychar[] = {
  '1','2','3','A',
  '4','5','6','D',
  '7','8','9','S',
  '*','0','#','M'
};

// 建立 BMK52T016 物件，參數為：中斷腳 pin 2，並使用 Wire（I2C）介面
BMK52T016 BMK52(2, &Wire);  

// 若使用 BMduino 板上的 Wire1 或 Wire2，可改以下方式：
// BMK52T016 BMK52(22, &Wire1); // 使用 Wire1，並使用中斷腳 22
// BMK52T016 BMK52(25, &Wire2); // 使用 Wire2，並使用中斷腳 25

//--------------------------------------
// 前置宣告各函式
void initKeyPad();        // 初始化 BMK52T016 模組
boolean setbuttondelay(); // 設定按鍵延遲時間
int readKey();            // 讀取被按下的按鍵編號 (1~16)
void listkeymask();       // 列出所有 keymask 的二進位格式
char inputKey();          // 回傳被按下按鍵對應的字元 (char)

//--------------------------------------

void initKeyPad() // 初始化 I2C 按鍵模組
{
    BMK52.begin();        // 啟動 BMK52T016 模組
    setbuttondelay();     // 設定每個按鍵的延遲時間，避免按鍵去彈跳問題
}

boolean setbuttondelay() // 設定每顆按鍵的延遲時間
{
    BMK52.setThreshold(buttondelay); // 呼叫模組函式設定按鍵的觸發門檻
}

// 顯示每顆按鍵對應的掩碼值（以二進位格式列印）
void listkeymask()
{
    int rkey = -1;
    for (int i = 0; i < 16; i++)
    {
        rkey = keymask[i];
        Serial.print("key Mask:(");
        Serial.print(rkey, BIN);  // 以二進位格式顯示掩碼
        Serial.print(")\n");
    }
}

// 讀取目前被按下的按鍵，並回傳其編號 (1~16)，若無按鍵則回傳 -1 或 -2
int readKey()
{
    uint16_t rkey = -1;

    // 檢查中斷腳是否為 LOW（代表有按鍵事件）
    if (BMK52.getINT() == 0)
    {
        rkey = BMK52.readKeyValue(); // 讀取 16-bit 的按鍵狀態

        // 比對哪一個按鍵被按下
        for (int i = 0; i < 16; i++)
        {
            // 使用 bit AND 與 keymask 比對特定按鍵是否為 1
            if ((rkey & keymask[i]) == keymask[i])
                return (i + 1); // 回傳被按下的按鍵編號（從 1 開始）
        }
    }
    else
    {
        return (-2); // 沒有觸發中斷事件（未按鍵）
    }

    return (-1); // 未偵測到任何按鍵
}

// 取得目前被按下的按鍵，並轉換成對應的字元
char inputKey()
{
    int rkey = -1;
    rkey = readKey(); // 先讀取被按下的按鍵編號

    if (rkey > 0)
    {
        return keychar[rkey - 1]; // 轉換成對應的字元回傳
    }
    else
    {
        return ('E'); // 錯誤或無按鍵，回傳 'E' 表示 Error / Empty
    }
}
