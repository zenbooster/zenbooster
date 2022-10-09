#pragma once
#include <time.h>

namespace RingBufferInItem
{
struct TRingBufferInItem
{
  time_t time;
  int delta;
  int theta;
  int alpha_lo;
  int alpha_hi;
  int beta_lo;
  int beta_hi;
  int gamma_lo;
  int gamma_md;
  int esense_att;
  int esense_med;

  /*String serialize(void)
  {
    String res = "time="+String(time)+
          ", d="+String(delta)+", t="+String(theta)+
          ", al="+String(alpha_lo)+", ah="+String(alpha_hi)+
          ", bl="+String(beta_lo)+", bh="+String(beta_hi)+
          ", gl="+String(gamma_lo)+", gm="+String(gamma_md);
    return res;
  }*/
};
}