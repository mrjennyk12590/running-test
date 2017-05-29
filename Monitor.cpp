#include "Monitor.h"

// コンストラクタ
Monitor::Monitor()
{
    sdCard = new SdCard();
    reset();
}

// デストラクタ
Monitor::~Monitor()
{
    delete sdCard;
}

//初期化処理
void Monitor::reset()
{
    x_ = 0;
    y_ = 0;
    ev3_lcd_fill_rect(x_, y_, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);
    ev3_lcd_set_font(EV3_FONT_SMALL);
}

//表示処理
void Monitor::display(const char* str)
{
    // モニターの表示エリアを超える場合は、モニターを初期化する
    if(y_ >= EV3_LCD_HEIGHT)
    {
        reset();
    }
    ev3_lcd_draw_string(str, x_, y_);
    sdCard->write(str);
    next();
}

// 次の行に移る
void Monitor::next()
{
    x_ = 0;
    y_ += 8;
}
