#include <stdio.h>
#include "ev3api.h"

class SdCard
{
private:
  FILE *file_;
  char fileBuf_[512];               // 一回に書き出しできる文字長を指定

public:
  SdCard();
  SdCard(char *path, char *mode);
  ~SdCard();

  void read();                      // 読み込み
  void write(const char * str);     // 書き込み
  void open(char *path, char *mode);
  void close();
};
