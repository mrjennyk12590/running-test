#include <stdio.h>
#include <string.h>
#include <time.h>
#include "SdCard.h"
#include "ev3api.h"

// コンストラクタ
SdCard::SdCard()
{
    // // 現在日時の取得
    // time_t timer;
    // struct tm *date;
    // char buf[256];
    //
    // time(&timer);
    // date = localtime(&timer);
    // sprintf(buf,"stecdan_debug%d%d%d",date->tm_year,date->tm_mon,date->tm_mday);

    open((char*)"/ev3rt/logs/stecdan_debug_yymmdd.log",(char*)"a+");
}
SdCard::SdCard(char *path, char *mode)
{
    open(path, mode);
}

// デストラクタ
SdCard::~SdCard()
{
    close();
}

void SdCard::read()
{
    // 処理未実装
}

void SdCard::write(const char *str)
{
    // バッファサイズより大きくなる場合は、一旦ファイルに書き出しする
    if(strlen(fileBuf_) + strlen(str) >= 512)
    {
        fprintf(file_, "%s", fileBuf_);
        memset(fileBuf_, '\0', strlen(fileBuf_));
    }

    // バッファ（メモリ）に書き出し
    sprintf(fileBuf_, "%s\n", strcat(fileBuf_,str));
    // fprintf(file_, "%s\n\r", str);
}

void SdCard::open(char *path, char *mode)
{
    file_ = fopen(path ,mode);
}

void SdCard::close()
{
    // クローズ時にファイルに書き出す
    fprintf(file_, "%s\n", fileBuf_);
    fclose(file_);
}
