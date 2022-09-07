#pragma once

namespace Noise
{
class TNoise
{
  private:
    static const int i2s_num = 0;
    static const int SAMPLE_RATE = 22050;//11025;
    static float level;

    //static void IRAM_ATTR timer0_ISR(void *ptr);
    static void task(void *p);

  public:
    static float MAX_NOISE_LEVEL;// = 5;

    TNoise();
    static float set_level(float lvl);
    static float get_level(void);
};
}
