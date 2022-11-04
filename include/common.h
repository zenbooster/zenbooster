#pragma once
#include <stdint.h>

namespace common {

//#define SOUND_DAC // вывод звука через ЦАП
//#define SOUND_I2S // вывод звука через I2S

/*#if defined(SOUND_DAC) && defined(SOUND_I2S)
#   error Разрешено использование только одного из макросов, SOUND_DAC или SOUND_I2S!
#endif
*/

/*#if !(defined(SOUND_DAC) || defined(SOUND_I2S))
#   error Какой способ вывода звука будет использоваться? Нужно указать один из макросов, SOUND_DAC или SOUND_I2S!
#endif*/

#define TWORKER_TASK_STACK_SIZE 2300
#define TWIFISTUFF_TASK_STACK_SIZE 7700
#define TBLUETOOTHSTUFF_TASK_STACK_SIZE 1000
//#define TBLUETOOTHSTUFF_TASK_STACK_SIZE 2500
#define TNOISE_TASK_STACK_SIZE 1000

#if defined(SOUND_DAC) || defined(SOUND_I2S)
#   define SOUND
#endif

#ifdef ARDUINO_ESP32_DEV
static const uint8_t  LED_BUILTIN = 2;

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
#else
#ifdef LILYGO_WATCH_2020_V2
#   define TFT_WIDTH 240
#   define TFT_HEIGHT 240

#   define TFT_MISO (gpio_num_t)0
#   define TFT_MOSI (gpio_num_t)19
#   define TFT_SCLK (gpio_num_t)18
#   define TFT_CS (gpio_num_t)5
#   define TFT_DC (gpio_num_t)27
#   define TFT_RST (gpio_num_t)-1
#   define TFT_BL (gpio_num_t)25

#   ifdef _VBAT
#       define PIN_BATTARY _VBAT
#   endif
#   define A0 36
#endif
#endif
#endif
}