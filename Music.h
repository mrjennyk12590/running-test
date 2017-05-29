#include "ev3api.h"
#include "Clock.h"

class Music
{
private:
  ev3api::Clock *clock_;
  
public:
  Music(uint8_t volume=5);
  ~Music();

  void playMusic();
  void playTone(uint16_t frequency=NOTE_C5, int32_t duration=1000);

};
