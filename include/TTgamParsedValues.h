#pragma once
#include <Arduino.h>
#include "TRingBufferInItem.h"
#include "TTgamParsedValues.h"

namespace TgamParsedValues
{
using namespace RingBufferInItem;

struct TTgamParsedValues: public TRingBufferInItem
{
  time_t time;
  bool is_has_batt;
  uint8_t batt;
  bool is_has_poor;
  uint8_t poor;
  bool is_has_eeg_power;

  const String serialize(void) const;

  private:
    static void add_string(String& dst, const String& src);
};
}