#include "ev3api.h"
#include "Clock.h"

class ClockTime : public ev3api::Clock
{
private:
  int x_;
  int y_;

public:
  ClockTime();
  ~ClockTime();
  void test();
};
