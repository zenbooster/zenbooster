#pragma once
#include <Arduino.h>
#include "TSingleton.h"
#include "TTask.h"

namespace Noise
{
using namespace Singleton;
using namespace Task;

#ifdef SOUND_DAC
typedef double numeric; // software
#else
typedef float numeric; // hardware
#endif

class TNoise: public TSingleton<TNoise>
{
  private:
    static numeric level;

    const char *get_class_name();

  #ifdef SOUND_DAC
    static const int SAMPLE_RATE_DAC = 11025;

    static void IRAM_ATTR timer0_ISR(void *ptr);
  #endif
  #ifdef SOUND_I2S
    static const int SAMPLE_RATE_I2S = 22050;
    static const int i2s_num = 0;
    static TTask *p_task;

    static void task_i2s(void *p);
  #endif
  public:
    static numeric MAX_NOISE_LEVEL;

    TNoise();
    ~TNoise();

    static numeric set_level(numeric lvl);
    static numeric get_level(void);
};
}
