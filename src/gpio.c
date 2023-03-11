#include <mruby.h>
#include <mruby/value.h>

#include "driver/gpio.h"
#include "driver/dac_oneshot.h"
#include "esp_adc/adc_oneshot.h"

#define GPIO_MODE_DEF_PULLUP (BIT3)
#define GPIO_MODE_DEF_PULLDOWN (BIT3)
#define GPIO_MODE_INPUT_PULLUP ((GPIO_MODE_INPUT)|(GPIO_MODE_DEF_PULLUP))
#define GPIO_MODE_INPUT_PULLDOWN ((GPIO_MODE_INPUT)|(GPIO_MODE_DEF_PULLDOWN))
#define GPIO_MODE_OUTPUT (GPIO_MODE_DEF_OUTPUT) 
#define GPIO_MODE_OUTPUT_OD ((GPIO_MODE_DEF_OUTPUT)|(GPIO_MODE_DEF_OD))
#define GPIO_MODE_INPUT_OUTPUT_OD ((GPIO_MODE_DEF_INPUT)|(GPIO_MODE_DEF_OUTPUT)|(GPIO_MODE_DEF_OD))
#define GPIO_MODE_INPUT_OUTPUT ((GPIO_MODE_DEF_INPUT)|(GPIO_MODE_DEF_OUTPUT))

static mrb_value
mrb_esp32_gpio_pin_mode(mrb_state *mrb, mrb_value self) {
  mrb_value pin, dir;

  mrb_get_args(mrb, "oo", &pin, &dir);

  if (!mrb_fixnum_p(pin) || !mrb_fixnum_p(dir)) {
    return mrb_nil_value();
  }

  esp_rom_gpio_pad_select_gpio(mrb_fixnum(pin));
  gpio_set_direction(mrb_fixnum(pin), mrb_fixnum(dir) & ~GPIO_MODE_DEF_PULLUP);

  if (mrb_fixnum(dir) & GPIO_MODE_DEF_PULLUP) {
    gpio_set_pull_mode(mrb_fixnum(pin), GPIO_PULLUP_ONLY);
  }

  return self;
}

static mrb_value
mrb_esp32_gpio_digital_write(mrb_state *mrb, mrb_value self) {
  mrb_value pin, level;

  mrb_get_args(mrb, "oo", &pin, &level);

  if (!mrb_fixnum_p(pin) || !mrb_fixnum_p(level)) {
    return mrb_nil_value();
  }

  gpio_set_level(mrb_fixnum(pin), mrb_fixnum(level));

  return self;
}

static mrb_value
mrb_esp32_gpio_analog_write(mrb_state *mrb, mrb_value self) {
  mrb_value ch, vol;

  mrb_get_args(mrb, "oo", &ch, &vol);

  if (!mrb_fixnum_p(ch) || !mrb_fixnum_p(vol)) {
    return mrb_nil_value();
  }
  
  // Handle
  dac_oneshot_handle_t chan_handle;
  
  // Configuration
  dac_oneshot_config_t chan_cfg = {
      .chan_id = mrb_fixnum(ch),
  };
  ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan_cfg, &chan_handle));
  
  // Write
  ESP_ERROR_CHECK(dac_oneshot_output_voltage(chan_handle, mrb_fixnum(vol)));
  
  return self;
}

static mrb_value
mrb_esp32_gpio_digital_read(mrb_state *mrb, mrb_value self) {
  mrb_value pin;

  mrb_get_args(mrb, "o", &pin);

  if (!mrb_fixnum_p(pin)) {
    return mrb_nil_value();
  }

  return mrb_fixnum_value(gpio_get_level(mrb_fixnum(pin)));
}

static mrb_value
mrb_esp32_gpio_analog_read(mrb_state *mrb, mrb_value self) {
  mrb_value ch;

  mrb_get_args(mrb, "o", &ch);

  if (!mrb_fixnum_p(ch)) {
    return mrb_nil_value();
  }
  
  // Handle
  adc_oneshot_unit_handle_t adc1_handle;
  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
  
  // Configuration. ADC_BITWIDTH_DEFAULT = 12
  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_11,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, mrb_fixnum(ch), &config));
  
  // Read and Delete
  int adc_result;
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, mrb_fixnum(ch), &adc_result));
  ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

  return mrb_fixnum_value(adc_result);
}

void
mrb_mruby_esp32_gpio_gem_init(mrb_state* mrb)
{
  struct RClass *esp32, *gpio, *constants;

  esp32 = mrb_define_module(mrb, "ESP32");

  gpio = mrb_define_module_under(mrb, esp32, "GPIO");
  mrb_define_module_function(mrb, gpio, "pinMode", mrb_esp32_gpio_pin_mode, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, gpio, "digitalWrite", mrb_esp32_gpio_digital_write, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, gpio, "digitalRead", mrb_esp32_gpio_digital_read, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, gpio, "analogWrite", mrb_esp32_gpio_analog_write, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, gpio, "analogRead", mrb_esp32_gpio_analog_read, MRB_ARGS_REQ(1));

  constants = mrb_define_module_under(mrb, gpio, "Constants");

#define define_const(SYM) \
  do { \
    mrb_define_const(mrb, constants, #SYM, mrb_fixnum_value(SYM)); \
  } while (0)

  define_const(GPIO_NUM_0);
  define_const(GPIO_NUM_1);
  define_const(GPIO_NUM_2);
  define_const(GPIO_NUM_3);
  define_const(GPIO_NUM_4);
  define_const(GPIO_NUM_5);
  define_const(GPIO_NUM_6);
  define_const(GPIO_NUM_7);
  define_const(GPIO_NUM_8);
  define_const(GPIO_NUM_9);
  define_const(GPIO_NUM_10);
  define_const(GPIO_NUM_11);
  define_const(GPIO_NUM_12);
  define_const(GPIO_NUM_13);
  define_const(GPIO_NUM_14);
  define_const(GPIO_NUM_15);
  define_const(GPIO_NUM_16);
  define_const(GPIO_NUM_17);
  define_const(GPIO_NUM_18);
  define_const(GPIO_NUM_19);

  define_const(GPIO_NUM_21);
  define_const(GPIO_NUM_22);
  define_const(GPIO_NUM_23);

  define_const(GPIO_NUM_25);
  define_const(GPIO_NUM_26);
  define_const(GPIO_NUM_27);

  define_const(GPIO_NUM_32);
  define_const(GPIO_NUM_33);
  define_const(GPIO_NUM_34);
  define_const(GPIO_NUM_35);
  define_const(GPIO_NUM_36);
  define_const(GPIO_NUM_37);
  define_const(GPIO_NUM_38);
  define_const(GPIO_NUM_39);
  define_const(GPIO_NUM_MAX);

  define_const(DAC_CHAN_0);
  define_const(DAC_CHAN_1);
  // Old versions of above. Deprecated.
  define_const(DAC_CHANNEL_1);
  define_const(DAC_CHANNEL_2);

  define_const(ADC_CHANNEL_0);
  define_const(ADC_CHANNEL_1);
  define_const(ADC_CHANNEL_2);
  define_const(ADC_CHANNEL_3);
  define_const(ADC_CHANNEL_4);
  define_const(ADC_CHANNEL_5);
  define_const(ADC_CHANNEL_6);
  define_const(ADC_CHANNEL_7);
  // Channel 8 and 9 only exist on ADC2.
  define_const(ADC_CHANNEL_8);
  define_const(ADC_CHANNEL_9);

  mrb_define_const(mrb, constants, "LOW", mrb_fixnum_value(0));
  mrb_define_const(mrb, constants, "HIGH", mrb_fixnum_value(1));

  mrb_define_const(mrb, constants, "INPUT",          mrb_fixnum_value(GPIO_MODE_INPUT));
  mrb_define_const(mrb, constants, "INPUT_OUTPUT",   mrb_fixnum_value(GPIO_MODE_INPUT_OUTPUT));
  mrb_define_const(mrb, constants, "OUTPUT",         mrb_fixnum_value(GPIO_MODE_OUTPUT));
  mrb_define_const(mrb, constants, "INPUT_PULLUP",   mrb_fixnum_value(GPIO_MODE_INPUT_PULLUP));
  mrb_define_const(mrb, constants, "INPUT_PULLDOWN", mrb_fixnum_value(GPIO_MODE_INPUT_PULLDOWN));
    
}

void
mrb_mruby_esp32_gpio_gem_final(mrb_state* mrb)
{
}
