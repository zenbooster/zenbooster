#include "TNoise.h"
#include "driver/timer.h"
#include "driver/dac.h"

namespace Noise
{
int TNoise::MAX_NOISE_LEVEL = 255;
int TNoise::level = 255;

void IRAM_ATTR TNoise::timer0_ISR(void *ptr)
{
  TNoise *p = (TNoise *)ptr;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  //Reset irq and set for next time
  TIMERG0.int_clr_timers.t0 = 1;
  TIMERG0.hw_timer[0].config.alarm_en = 1;

  char val = p->level ? esp_random() % p->level : 0;
  dac_output_voltage(DAC_CHANNEL_1, val);
}

TNoise::TNoise()
{
  level = MAX_NOISE_LEVEL;
  ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_1));

  timer_config_t config = {
    .alarm_en = TIMER_ALARM_EN, // Включить прерывание Alarm
    .counter_en = TIMER_PAUSE, // Состояние - пауза
    .intr_type = TIMER_INTR_LEVEL, // Прерывание по уровню
    .counter_dir = TIMER_COUNT_UP, // Считать вверх
    .auto_reload = TIMER_AUTORELOAD_EN,//1, // Автоматически перезапускать счет
    .divider = 8, // Предделитель
  };

  // Применить конфиг
  ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &config));
  // Установить начальное значение счетчика
  ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL));
  // Установить значение счетчика для срабатывания прерывания Alarm
  ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_BASE_CLK / config.divider / SAMPLE_RATE));
  // Разрешить прерывания
  ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
  // Зарегистрировать обработчик прерывания
  timer_isr_register(TIMER_GROUP_0, TIMER_0, timer0_ISR, this, ESP_INTR_FLAG_IRAM, NULL);
  // Запустить таймер
  timer_start(TIMER_GROUP_0, TIMER_0);
}

int TNoise::set_level(int lvl)
{
  int res = level;
  level = lvl;

  return res;
}

int TNoise::get_level(void)
{
  return level;
}
}
