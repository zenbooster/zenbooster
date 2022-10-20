#pragma once

namespace Noise
{
#ifdef SOUND_DAC
typedef double numeric; // software
#else
typedef float numeric; // hardware
#endif

class TNoise
{
  private:
  #ifdef SOUND_DAC
    static const int SAMPLE_RATE_DAC = 11025;
  #endif
  #ifdef SOUND_I2S
    static const int SAMPLE_RATE_I2S = 22050;
  #endif
    static numeric level;
  #ifdef SOUND_DAC
    static void IRAM_ATTR timer0_ISR(void *ptr);
  #endif
  #ifdef SOUND_I2S
    static const int i2s_num = 0;
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
