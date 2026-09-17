// ================================================================
// 檔案名稱：OledLib.h
// 描述：OLED 顯示模組自訂函式庫
// 功能：提供 BMD31M090 OLED 顯示模組（128x64）的完整控制功能
//       包含文字顯示、圖形繪製、螢幕控制、捲動效果等
// 通訊介面：I2C (Wire1)
// ================================================================

// ================================================================
// =============== 全域常數與變數宣告區 ===============
// ================================================================

// 定義 BMD31M090 顯示模組的寬度和高度（單位：像素）
#define BMD31M090_WIDTH   128        // OLED 螢幕寬度：128 像素
#define BMD31M090_HEIGHT  64         // OLED 螢幕高度：64 像素

// 設定 BMD31M090 顯示模組的 I2C 地址
// 根據電路圖和 OLED 模組的 A0 接腳設定選擇對應地址
#define BMD31M090_ADDRESS 0x3C       // 預設 I2C 地址：0x3C (A0 接地)
// #define BMD31M090_ADDRESS 0x3D   // 可選 I2C 地址：0x3D (A0 接電源)

uint8_t t = ' ';  // 全域變數 t，用於 ASCII 字元測試，初始化為空格字元

// ================================================================
// =============== 函式庫引入區 ===============
// ================================================================

// 引入 BMD31M090 OLED 顯示模組的函式庫（底層驅動）
#include "BMD31M090.h"

// 引入 BestModule 公司標誌的點陣圖資料
#include "BestModuleLogo.h"

// 注意：已註解掉的函式庫，可根據需求啟用
// #include "Bitmap.h"     // 引入位圖相關的函式庫（如有需要）

// ================================================================
// =============== OLED 物件實例化區 ===============
// ================================================================

/*
建立 BMD31M090 OLED 顯示物件，命名為 BMD31
參數說明：
  - BMD31M090_WIDTH: OLED 寬度 (128)
  - BMD31M090_HEIGHT: OLED 高度 (64)
  - &Wire1: 使用第一組 I2C 介面（本程式選用）

注意：根據開發板選擇適當的 I2C 介面：
  1. &Wire   - Arduino 預設 I2C（通常 SDA:A4, SCL:A5）
  2. &Wire1  - 第一組額外 I2C（BMduino 開發板常用）
  3. &Wire2  - 第二組額外 I2C（如開發板支援）
*/
BMD31M090 BMD31(BMD31M090_WIDTH, BMD31M090_HEIGHT, &Wire1);

// ================================================================
// =============== 函式宣告區 ===============
// ================================================================

// ---------- 基本顯示控制函式 ----------
void initOled();                    // 初始化 OLED 128x64 顯示模組
void drawPicture(int x, int y, const uint8_t *pp, int width, int height);  // 繪製點陣圖
void clearScreen();                 // 清除螢幕內容
void updateScreen();                // 更新螢幕顯示（將緩衝區內容輸出到硬體）
void setFont(const unsigned char* font);  // 設定顯示字型

// ---------- 文字顯示函式 ----------
void printText(int x, int y, uint8_t *str);    // 在指定位置顯示文字
void printChar(int x, int y, char str);        // 在指定位置顯示單一字元
void printNumber(int x, int y, int num);       // 在指定位置顯示整數
void printFloat(int x, int y, float num);      // 在指定位置顯示浮點數

// ---------- 圖形繪製函式 ----------
void drawPoint(int x, int y, int pixelColor);  // 在指定位置繪製單一點
void drawLine(int x1, int y1, int x2, int y2, int pixelColor);  // 繪製直線
void drawfastVline(int x, int y, int width, int pixelColor);    // 快速繪製垂直線
void drawfastHline(int x, int y, int width, int pixelColor);    // 快速繪製水平線
void drawVline(int x, int y, int width, int pixelColor);        // 繪製垂直線
void drawHline(int x, int y, int width, int pixelColor);        // 繪製水平線
void drawBox(int x1, int y1, int x2, int y2, int pixelColor);   // 繪製矩形框

// ---------- 捲動控制函式 ----------
void scrollRight(int x, int y, int speed, int dir);  // 向右捲動
void scrollLeft(int x, int y, int speed, int dir);   // 向左捲動
void stopScroll();                                   // 停止捲動

// ---------- 螢幕模式控制函式 ----------
void setsaveMode();        // 設定省電模式（降低亮度）
void setlightMode();       // 設定正常亮度模式
void setdisplayInverse();  // 設定螢幕反白模式
void setdisplayNormal();   // 設定螢幕正常模式

// ---------- 應用層顯示函式 ----------
void showDeviceonOled(String ss, int row);   // 顯示裝置 ID
void showTitleonOled(String ss, int row);    // 顯示標題文字
void showIPonOled(String ss, int row);       // 顯示 IP 位址
void showMACeonOled(String ss, int row);     // 顯示 MAC 位址
void showSSIDeonOled(String ss, int row);    // 顯示 WiFi SSID
void showMsgonOled(String ss, int row);      // 顯示一般訊息

// ---------- 測試展示函式 ----------
void test_drawString_6x8();              // 測試 6x8 字型顯示
void test_drawString_8x16();             // 測試 8x16 字型顯示
void test_drawString_drawChar_drawNum(); // 測試字串、字元、數字顯示
void test_drawPixel();                   // 測試像素繪製
void test_drawFastHLine_drawFastVLine(); // 測試水平/垂直線繪製
void test_drawBitmap();                  // 測試點陣圖顯示
void test_variousScroll();               // 測試捲動效果
void test_invertDisplay();               // 測試反白顯示
void test_dim();                         // 測試亮度調整

// ================================================================
// =============== 函式實作區 ===============
// ================================================================

// ---------------------------------------------------------------
// 函式名稱：initOled()
// 功能：初始化 BMD31M090 OLED 顯示模組
// 參數：無
// 傳回值：無
// 流程：
//   1. 呼叫 BMD31.begin() 初始化 OLED，傳入 I2C 地址
//   2. 延遲 100ms 確保初始化完成
//   3. 序列埠輸出初始化成功訊息
// ---------------------------------------------------------------
void initOled()
{
    // 初始化 OLED 顯示模組，使用指定的 I2C 地址
    BMD31.begin(BMD31M090_ADDRESS);
    
    // 建議的初始化延遲時間，確保硬體穩定
    delay(100);
    
    // 序列埠輸出初始化成功訊息（用於偵錯）
    Serial.println("init OLED12864 OK");
}

// ---------------------------------------------------------------
// 函式名稱：setFont()
// 功能：設定 OLED 顯示字型
// 參數：
//   - font: 字型指標，可選擇以下預設字型：
//     * FontTable_6X8  : 6×8 像素字型（小字）
//     * FontTable_8X16 : 8×16 像素字型（標準）
//     * FontTable_16X32: 16×32 像素字型（大字）
//     * FontTable_32X64: 32×64 像素字型（特大）
// 傳回值：無
// ---------------------------------------------------------------
void setFont(const unsigned char* font)
{
    BMD31.setFont(font);
}

// ---------------------------------------------------------------
// 函式名稱：updateScreen()
// 功能：將顯示緩衝區的內容輸出到 OLED 硬體螢幕
// 參數：無
// 傳回值：無
// 說明：所有繪圖操作都在記憶體緩衝區進行，需呼叫此函式才會實際顯示
// ---------------------------------------------------------------
void updateScreen()
{
    BMD31.display();
}

// ---------------------------------------------------------------
// 函式名稱：printText()
// 功能：在指定座標位置顯示文字字串
// 參數：
//   - x: X 座標（0-127，從左到右）
//   - y: Y 座標（0-7，從上到下，以行為單位）
//   - str: 要顯示的文字字串
// 傳回值：無
// 注意：座標系統以字元行為單位，不是像素
// ---------------------------------------------------------------
void printText(int x, int y, String str)
{
    // 呼叫底層函式繪製字串
    // str.c_str() 將 String 轉換為 C 風格字串
    BMD31.drawString((uint8_t)x, (uint8_t)y, (uint8_t*)str.c_str());
}

// ---------------------------------------------------------------
// 函式名稱：printChar()
// 功能：在指定座標位置顯示單一字元
// 參數：
//   - x: X 座標（0-127）
//   - y: Y 座標（0-7）
//   - str: 要顯示的單一字元
// 傳回值：無
// ---------------------------------------------------------------
void printChar(int x, int y, char str)
{
    BMD31.drawChar((uint8_t)x, (uint8_t)y, (uint8_t)str);
}

// ---------------------------------------------------------------
// 函式名稱：printNumber()
// 功能：在指定座標位置顯示整數
// 參數：
//   - x: X 座標（0-127）
//   - y: Y 座標（0-7）
//   - num: 要顯示的整數值
// 傳回值：無
// 說明：自動計算數字位數並正確顯示
// ---------------------------------------------------------------
void printNumber(int x, int y, int num)
{
    // 參數說明：x, y, 數值, 數字位數
    BMD31.drawNum((uint8_t)x, (uint8_t)y, (uint32_t)num, (uint8_t)(String(num).length()));
}

// ---------------------------------------------------------------
// 函式名稱：printFloat()
// 功能：在指定座標位置顯示浮點數
// 參數：
//   - x: X 座標（0-127）
//   - y: Y 座標（0-7）
//   - num: 要顯示的浮點數值
// 傳回值：無
// 說明：實際上是將浮點數轉為字串後顯示
// ---------------------------------------------------------------
void printFloat(int x, int y, float num)
{
    // 將浮點數轉換為字串後顯示
    printText(x, y, String(num));
}

// ---------------------------------------------------------------
// 函式名稱：drawPoint()
// 功能：在指定像素位置繪製單一點
// 參數：
//   - x: X 座標（0-127）
//   - y: Y 座標（0-63）
//   - pixelColor: 像素顏色
//     0 (pixelColor_BLACK)  : 黑色
//     1 (pixelColor_WHITE)  : 白色
//     2 (pixelColor_INVERSE): 反轉顏色
// 傳回值：無
// ---------------------------------------------------------------
void drawPoint(int x, int y, int pixelColor)
{
    BMD31.drawPixel((uint8_t)x, (uint8_t)y, (uint8_t)pixelColor);
    BMD31.display();  // 立即更新顯示
}

// ---------------------------------------------------------------
// 函式名稱：drawLine()
// 功能：在兩點之間繪製直線
// 參數：
//   - x1, y1: 起點座標
//   - x2, y2: 終點座標
//   - pixelColor: 線條顏色（同上）
// 傳回值：無
// ---------------------------------------------------------------
void drawLine(int x1, int y1, int x2, int y2, int pixelColor)
{
    BMD31.drawLine((uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2, (uint8_t)pixelColor);
    BMD31.display();
}

// ---------------------------------------------------------------
// 函式名稱：drawfastVline()
// 功能：快速繪製垂直線（效能較佳）
// 參數：
//   - x, y: 起點座標
//   - width: 線條長度（像素）
//   - pixelColor: 線條顏色
// 傳回值：無
// ---------------------------------------------------------------
void drawfastVline(int x, int y, int width, int pixelColor)
{
    BMD31.drawFastVLine((uint8_t)x, (uint8_t)y, (uint8_t)width, (uint8_t)pixelColor);
    BMD31.display();
}

// ---------------------------------------------------------------
// 函式名稱：drawfastHline()
// 功能：快速繪製水平線（效能較佳）
// 參數：
//   - x, y: 起點座標
//   - width: 線條長度（像素）
//   - pixelColor: 線條顏色
// 傳回值：無
// ---------------------------------------------------------------
void drawfastHline(int x, int y, int width, int pixelColor)
{
    BMD31.drawFastHLine((uint8_t)x, (uint8_t)y, (uint8_t)width, (uint8_t)pixelColor);
    BMD31.display();
}

// ---------------------------------------------------------------
// 函式名稱：drawBox()
// 功能：繪製矩形框（空心矩形）
// 參數：
//   - x1, y1: 矩形左上角座標
//   - x2, y2: 矩形右下角座標
//   - pixelColor: 線條顏色
// 傳回值：無
// 說明：透過繪製四條直線組成矩形框
// ---------------------------------------------------------------
void drawBox(int x1, int y1, int x2, int y2, int pixelColor)
{
    // 繪製上邊線
    BMD31.drawLine((uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y1, (uint8_t)pixelColor);
    // 繪製右邊線
    BMD31.drawLine((uint8_t)x2, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2, (uint8_t)pixelColor);
    // 繪製左邊線
    BMD31.drawLine((uint8_t)x1, (uint8_t)y1, (uint8_t)x1, (uint8_t)y2, (uint8_t)pixelColor);
    // 繪製下邊線
    BMD31.drawLine((uint8_t)x1, (uint8_t)y2, (uint8_t)x2, (uint8_t)y2, (uint8_t)pixelColor);
    BMD31.display();
}

// ---------------------------------------------------------------
// 函式名稱：clearScreen()
// 功能：清除 OLED 螢幕所有內容
// 參數：無
// 傳回值：無
// 流程：
//   1. 清除顯示緩衝區
//   2. 更新螢幕顯示
// ---------------------------------------------------------------
void clearScreen()
{
    BMD31.clearDisplay();  // 清除緩衝區
    BMD31.display();       // 更新到硬體
}

// ---------------------------------------------------------------
// 函式名稱：drawPicture()
// 功能：在指定位置繪製點陣圖（白色顯示）
// 參數：
//   - x, y: 圖片左上角座標
//   - pp: 點陣圖資料指標
//   - width: 圖片寬度（像素）
//   - height: 圖片高度（像素）
// 傳回值：無
// 說明：先清除螢幕，再繪製白色圖片
// ---------------------------------------------------------------
void drawPicture(int x, int y, const uint8_t *pp, int width, int height)
{
    BMD31.clearDisplay();
    // 繪製白色點陣圖
    BMD31.drawBitmap((uint8_t)x, (uint8_t)y, pp, (uint8_t)width, (uint8_t)height, pixelColor_WHITE);
    BMD31.display();
}

// ---------------------------------------------------------------
// 函式名稱：setsaveMode()
// 功能：設定省電模式（降低螢幕亮度）
// 參數：無
// 傳回值：無
// 說明：適合用於電池供電或需要省電的場景
// ---------------------------------------------------------------
void setsaveMode()
{
    // TRUE 表示降低亮度（省電模式）
    BMD31.dim(TRUE);
}

// ---------------------------------------------------------------
// 函式名稱：setlightMode()
// 功能：設定正常亮度模式
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void setlightMode()
{
    // FALSE 表示正常亮度
    BMD31.dim(FALSE);
}

// ---------------------------------------------------------------
// 函式名稱：setdisplayInverse()
// 功能：設定螢幕反白模式（白底黑字）
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void setdisplayInverse()
{
    // TRUE 表示反白顯示
    BMD31.invertDisplay(TRUE);
}

// ---------------------------------------------------------------
// 函式名稱：setdisplayNormal()
// 功能：設定螢幕正常模式（黑底白字）
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void setdisplayNormal()
{
    // FALSE 表示正常顯示
    BMD31.invertDisplay(FALSE);
}

// ---------------------------------------------------------------
// 函式名稱：scrollRight()
// 功能：啟動向右捲動效果
// 參數：
//   - x: 起始列位址（0-7）
//   - y: 結束列位址（0-7）
//   - speed: 捲動速度（參見下方常數定義）
//   - dir: 垂直捲動方向
// 傳回值：無
// 捲動速度常數：
//   SCROLL_2FRAMES, SCROLL_3FRAMES, SCROLL_4FRAMES,
//   SCROLL_5FRAMES, SCROLL_25FRAMES, SCROLL_64FRAMES,
//   SCROLL_128FRAMES, SCROLL_256FRAMES
// 垂直方向常數：
//   SCROLLV_NONE, SCROLLV_TOP, SCROLLV_BOTTOM
// ---------------------------------------------------------------
void scrollRight(int x, int y, int speed, int dir)
{
    BMD31.startScrollRight((uint8_t)x, (uint8_t)y, (uint8_t)speed, (uint8_t)dir);
}

// ---------------------------------------------------------------
// 函式名稱：scrollLeft()
// 功能：啟動向左捲動效果
// 參數：同 scrollRight()
// 傳回值：無
// ---------------------------------------------------------------
void scrollLeft(int x, int y, int speed, int dir)
{
    BMD31.startScrollLeft((uint8_t)x, (uint8_t)y, (uint8_t)speed, (uint8_t)dir);
}

// ---------------------------------------------------------------
// 函式名稱：stopScroll()
// 功能：停止所有捲動效果
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void stopScroll()
{
    BMD31.stopScroll();
}

// ---------------------------------------------------------------
// 函式名稱：showTitleonOled()
// 功能：顯示標題文字於 OLED 指定行
// 參數：
//   - ss: 要顯示的文字字串
//   - row: 顯示的行數（0-7）
// 傳回值：無
// 流程：
//   1. 先清除該行內容（顯示空白）
//   2. 顯示新的標題文字
//   3. 序列埠輸出偵錯訊息
// ---------------------------------------------------------------
void showTitleonOled(String ss, int row)
{
    // 先清除該行內容（顯示 15 個空格）
    printText(0, row, "              ");
    
    // 顯示新的標題文字
    printText(0, row, ss);
    
    // 序列埠輸出偵錯訊息
    Serial.print("Title on OLED:(");
    Serial.print(ss);
    Serial.print(")\n");
}

// ---------------------------------------------------------------
// 函式名稱：showIPonOled()
// 功能：顯示 IP 位址於 OLED 指定行
// 參數：
//   - ss: IP 位址字串
//   - row: 顯示的行數
// 傳回值：無
// ---------------------------------------------------------------
void showIPonOled(String ss, int row)
{
    printText(0, row, "              ");
    printText(0, row, ss);
    Serial.print("IP Address on OLED:(");
    Serial.print(ss);
    Serial.print(")\n");
}

// ---------------------------------------------------------------
// 函式名稱：showMsgonOled()
// 功能：顯示一般訊息於 OLED 指定行
// 參數：
//   - ss: 訊息字串
//   - row: 顯示的行數
// 傳回值：無
// 說明：這是應用層最常用的顯示函式
// ---------------------------------------------------------------
void showMsgonOled(String ss, int row)
{
    printText(0, row, "              ");
    printText(0, row, ss);
    Serial.print("Message on OLED:(");
    Serial.print(ss);
    Serial.print(")\n");
}

// ================================================================
// =============== 測試函式區 ===============
// ================================================================

// ---------------------------------------------------------------
// 函式名稱：test_invertDisplay()
// 功能：測試螢幕反白與正常模式切換
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void test_invertDisplay(void)
{
    BMD31.invertDisplay(TRUE);   // 反白模式
    delay(500);
    BMD31.invertDisplay(FALSE);  // 正常模式
    delay(500);
    // 重複一次
    BMD31.invertDisplay(TRUE);
    delay(500);
    BMD31.invertDisplay(FALSE);
    delay(500);
}

// ---------------------------------------------------------------
// 函式名稱：test_dim()
// 功能：測試亮度調整功能
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void test_dim(void)
{
    BMD31.dim(TRUE);    // 降低亮度（省電模式）
    delay(500);
    BMD31.dim(FALSE);   // 恢復正常亮度
    delay(500);
    // 重複一次
    BMD31.dim(TRUE);
    delay(500);
    BMD31.dim(FALSE);
    delay(500);
}

// ---------------------------------------------------------------
// 函式名稱：test_drawString_6x8()
// 功能：測試 6x8 小字型顯示
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void test_drawString_6x8(void)
{
    BMD31.clearDisplay();
    BMD31.display();
    
    // 設定為 6x8 字型
    BMD31.setFont(FontTable_6X8);
    
    // 計算置中位置
    uint8_t col = (128 - (6 * sizeof("Hello World!"))) / 2;
    
    // 在每一行顯示 "Hello World!"
    for (uint8_t row = 0; row < 8; row++)
        BMD31.drawString(col, row, (u8*)"Hello World!");
    
    delay(500);
}

// ---------------------------------------------------------------
// 函式名稱：test_drawString_8x16()
// 功能：測試 8x16 標準字型顯示
// 參數：無
// 傳回值：無
// ---------------------------------------------------------------
void test_drawString_8x16(void)
{
    BMD31.clearDisplay();
    BMD31.display();
    
    // 設定為 8x16 字型
    BMD31.setFont(FontTable_8X16);
    
    // 計算置中位置
    uint8_t col = (128 - (8 * sizeof("Hello World!"))) / 2;
    
    // 每隔一行顯示（因為字型高度為 16 像素）
    for (uint8_t row = 0; row < 8; row += 2)
        BMD31.drawString(col, row, (u8*)"Hello World!");
    
    delay(500);
}

// ---------------------------------------------------------------
// 函式名稱：test_drawBitmap()
// 功能：測試點陣圖顯示功能
// 參數：無
// 傳回值：無
// 說明：顯示 BestModule 公司標誌，測試不同顏色模式
// ---------------------------------------------------------------
void test_drawBitmap()
{
    // 顯示白色標誌
    BMD31.clearDisplay();
    BMD31.drawBitmap(0, 0, BestModule_LOGO, 128, 64, pixelColor_WHITE);
    BMD31.display();
    delay(300);
    
    // 顯示黑色標誌
    BMD31.drawBitmap(0, 0, BestModule_LOGO, 128, 64, pixelColor_BLACK);
    BMD31.display();
    delay(300);
    
    // 顯示反轉顏色標誌
    BMD31.drawBitmap(0, 0, BestModule_LOGO, 128, 64, pixelColor_INVERSE);
    BMD31.display();
    delay(300);
}
