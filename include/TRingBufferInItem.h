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
};
}