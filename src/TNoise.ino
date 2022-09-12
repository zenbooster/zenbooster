#include "TNoise.h"
#include "driver/timer.h"
#ifdef SOUND_DAC
# include "driver/dac.h"
#endif
#ifdef SOUND_I2S
# include "driver/i2s.h"
# include "common.h"
#endif

namespace Noise
{
numeric TNoise::MAX_NOISE_LEVEL = 0.1;
numeric TNoise::level = TNoise::MAX_NOISE_LEVEL;

#ifdef SOUND_DAC
void TNoise::timer0_ISR(void *ptr)
{
  TNoise *pthis = static_cast<TNoise *>(ptr);

  //Reset irq and set for next time
  TIMERG0.int_clr_timers.t0 = 1;
  TIMERG0.hw_timer[0].config.alarm_en = 1;

  uint8_t val = esp_random() & 0xff;
  val = pthis->level ? ((val * pthis->level) / 100) : 0;
  dac_output_voltage(DAC_CHANNEL_1, val);
}
#endif

#ifdef SOUND_I2S
void TNoise::task_i2s(void *p)
{
  TNoise *pthis = static_cast<TNoise *>(p);

  uint16_t buf[128]; // Было 16. Посмотрим, останутся ли подвисания звука...

  for(;;)
  {
    uint16_t *pb = buf;
    for(int i = 0; i < sizeof(buf) / sizeof(uint16_t); i++)
    {
      uint16_t val = esp_random() & 0xffff;
      val = pthis->level ? ((val * pthis->level) / 100) : 0;

      *pb++ = val;
    }
    
    i2s_write_bytes((i2s_port_t)i2s_num, (const char *)&buf, sizeof(buf), 100);
    yield();
  }
}
#endif

TNoise::TNoise()
{
  //level = MAX_NOISE_LEVEL;
#ifdef SOUND_DAC
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
  ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_BASE_CLK / config.divider / SAMPLE_RATE_DAC));
  // Разрешить прерывания
  ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));
  // Зарегистрировать обработчик прерывания
  timer_isr_register(TIMER_GROUP_0, TIMER_0, timer0_ISR, this, ESP_INTR_FLAG_IRAM, NULL);
  // Запустить таймер
  timer_start(TIMER_GROUP_0, TIMER_0);
#endif

#ifdef SOUND_I2S
  pinMode(PIN_I2S_SD, OUTPUT);
  digitalWrite(PIN_I2S_SD, HIGH);

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE_I2S,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      //.bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64   //Interrupt level 1
      };
      
  i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_I2S_BCK, //this is BCK pin
      .ws_io_num = PIN_I2S_LRCK, // this is LRCK pin
      .data_out_num = PIN_I2S_DATA, // this is DATA output pin
      .data_in_num = -1   //Not used
  };

  i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)i2s_num, &pin_config);
  //set sample rates of i2s to sample rate of wav file
  i2s_set_sample_rates((i2s_port_t)i2s_num, SAMPLE_RATE_I2S); 

  xTaskCreatePinnedToCore(task_i2s, "TNoise::task_i2s", 1200, this,
    (tskIDLE_PRIORITY + 2), NULL, portNUM_PROCESSORS - 2);
#endif
}

numeric TNoise::set_level(numeric lvl)
{
  numeric res = level;
  level = lvl;

  return res;
}

numeric TNoise::get_level(void)
{
  return level;
}
}
