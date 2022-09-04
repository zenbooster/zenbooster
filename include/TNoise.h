#pragma once

namespace Noise
{
class TNoise
{
  private:
    static const int SAMPLE_RATE = 11025;
    static int level;

    static void IRAM_ATTR timer0_ISR(void *ptr);

  public:
    static int MAX_NOISE_LEVEL;// = 5;

    TNoise();
    static int set_level(int lvl);
    static int get_level(void);
};
}
