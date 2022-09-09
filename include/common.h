#pragma once

namespace common {

//#define SOUND_DAC // вывод звука через ЦАП
//#define SOUND_I2S // вывод звука через I2S

/*#if defined(SOUND_DAC) && defined(SOUND_I2S)
#   error Разрешено использование только одного из макросов, SOUND_DAC или SOUND_I2S!
#endif
*/

#if !(defined(SOUND_DAC) || defined(SOUND_I2S))
#   error Какой способ вывода звука будет использоваться? Нужно указать один из макросов, SOUND_DAC или SOUND_I2S!
#endif

#ifdef ARDUINO_ESP32_DEV
#   define PIN_BTN 27
#   define PIN_LED_R 14
#   define PIN_LED_G 12
#   define PIN_LED_B 13

#   ifdef SOUND_I2S
#       define PIN_I2S_SD 17
#       define PIN_I2S_BCK 19
#       define PIN_I2S_LRCK 21
#       define PIN_I2S_DATA 18
#   endif
#else
#ifdef ARDUINO_TTGO_T18_V3
#   define PIN_BTN 34
#   define PIN_LED_R 32
#   define PIN_LED_G 33
#   define PIN_LED_B 25

#   ifdef SOUND_I2S
#       define PIN_I2S_SD 13
#       define PIN_I2S_BCK 14
#       define PIN_I2S_LRCK 27
#       define PIN_I2S_DATA 12
#   endif

#   define PIN_BATTARY _VBAT
#endif
#endif
}