/*****************************************************************
File:         BMduino_getMotionStatus.ino
Description:  1.使用 Wire1 介面與 BMS81M001 通訊。
              2.使用硬體序列埠（波特率 115200）與序列埠監視器通訊。
              3.當偵測到動作時，列印 "Motion detected!"。
******************************************************************/

#include "BMS81M001.h"          // 引入 BMS81M001 震動感測器函式庫
BMS81M001 WakeOnShake(22,&Wire1);      // 建立 BMS81M001 物件，並指定中斷腳位為 8

uint8_t thr;        // 用於儲存動作偵測的門檻值（靈敏度）
uint8_t dur;        // 用於儲存動作持續時間設定
uint8_t halt_delay; // 用於儲存空閒模式延遲設定

void setup()
{
    WakeOnShake.begin();         // 初始化 BMS81M001 模組
    Serial.begin(115200);        // 初始化序列埠通訊，波特率設為 115200（用於與電腦序列埠監視器通訊）
    delay(200);                  // 等待模組初始化完成，避免讀取錯誤

    // 嘗試讀取並顯示模組韌體版本
    if (WakeOnShake.getFWVer())
    {
        Serial.print("Firmware version :");
        Serial.println(WakeOnShake.getFWVer(), HEX); // 以十六進位格式顯示韌體版本
    }
    else
    {
        Serial.println("Firmware reading fail!");    // 若讀取失敗，顯示錯誤訊息
    }

    // 嘗試讀取模組的震動偵測參數設定（門檻值、持續時間、延遲時間）
    if (!(WakeOnShake.getParameterSetting(thr, dur, halt_delay))) // 若讀取成功，函式回傳 0
    {
        // 顯示讀取到的參數值
        Serial.print("Threshold=");
        Serial.print(thr);
        Serial.print(" Duration=");
        Serial.print(dur);
        Serial.print(" Delay=");
        Serial.println(halt_delay);
        Serial.println("Module is OK!");             // 模組運作正常
    }
    else
    {
        Serial.println("Motion reading fail!");      // 若讀取失敗，顯示錯誤訊息
    }
}

uint32_t counter = 0; // 計數器，用於記錄偵測到動作的次數

void loop()
{
    // 檢查模組狀態是否為 0（通常代表模組就緒）
    if (WakeOnShake.getStatus() == 0)
    {
        // 檢查是否偵測到震動
        if (WakeOnShake.getShakeStatus())
        {
            counter++; // 計數器增加
            // 顯示偵測到動作的訊息及目前計數
            Serial.println("Motion detected!  " + (String)counter);
        }
    }
    // 注意：此處無延遲，程式將持續高速檢查狀態，可能導致序列埠輸出過於頻繁
}