#pragma once
#include <time.h>

namespace TgamParsedValues {class TTgamParsedValues;}

namespace RingBufferInItem
{
using namespace TgamParsedValues;

struct TRingBufferInItem
{
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

  TRingBufferInItem& operator=(const TTgamParsedValues& other);
};
}