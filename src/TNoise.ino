#include "TNoise.h"
#include "driver/timer.h"
#include "driver/dac.h"
#include "driver/i2s.h"

namespace Noise
{
float TNoise::MAX_NOISE_LEVEL = 0.1;
float TNoise::level = TNoise::MAX_NOISE_LEVEL;

/*void TNoise::timer0_ISR(void *ptr)
{
  TNoise *p = (TNoise *)ptr;

  //Reset irq and set for next time
  TIMERG0.int_clr_timers.t0 = 1;
  TIMERG0.hw_timer[0].config.alarm_en = 1;

  char val = p->level ? esp_random() % p->level : 0;
  dac_output_voltage(DAC_CHANNEL_1, val);
}*/

void TNoise::task(void *p)
{
  TNoise *pthis = static_cast<TNoise *>(p);
  uint16_t buf[512];


  for(;;)
  {
    uint16_t *pb = buf;
    for(int i = 0; i < sizeof(buf) / sizeof(uint16_t); i++)
    {
      uint16_t val = esp_random() & 0xffff;
      val = pthis->level ? ((val * pthis->level) / 100) : 0;

      *pb++ = val;
    }
    
    //i2s_write_bytes((i2s_port_t)i2s_num, (const char *)&val, sizeof(uint16_t), 100);
    i2s_write_bytes((i2s_port_t)i2s_num, (const char *)&buf, sizeof(buf), 100);
    yield();
  }
}

TNoise::TNoise()
{
  //level = MAX_NOISE_LEVEL;

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      //.bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64   //Interrupt level 1
      };
      
  i2s_pin_config_t pin_config = {
      .bck_io_num = 14, //this is BCK pin
      .ws_io_num = 27, // this is LRCK pin
      .data_out_num = 12, // this is DATA output pin
      .data_in_num = -1   //Not used
  };

  i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)i2s_num, &pin_config);
  //set sample rates of i2s to sample rate of wav file
  i2s_set_sample_rates((i2s_port_t)i2s_num, SAMPLE_RATE); 

  //ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_1));

  /*for(;;)
  {
    //char val = level ? esp_random() % level : 0;
    uint32_t val = esp_random() & 0xffff;
    i2s_write_bytes((i2s_port_t)i2s_num, (const char *)&val, sizeof(uint32_t), 100);
  }*/

  /*timer_config_t config = {
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
  */
   int res = xTaskCreatePinnedToCore(task, "TNoise::task", 2000, this,
    (tskIDLE_PRIORITY + 2), NULL, portNUM_PROCESSORS - 2);
}

float TNoise::set_level(float lvl)
{
  float res = level;
  level = lvl;

  return res;
}

float TNoise::get_level(void)
{
  return level;
}
}
