#ifndef C2M_LED_H_
#define C2M_LED_H_

#include <stdint.h>
#include <string.h>
#include <Arduino.h>
#include "C2M_config.h"
#include "C2M_options.h"
#include "C2M_gpio.h"
#include "util/util_math.h"
#include "util/util_macros.h"

enum LED_CHANNEL {
  LED_CHANNEL_A, 
  LED_CHANNEL_B, 
  LED_CHANNEL_C, 
  LED_CHANNEL_D, 
  LED_CHANNEL_E, 
  LED_CHANNEL_X, 
  LED_CHANNEL_LAST
};
    
namespace C2M {

class LEDs {
public:

  static void Init();
  static void Update();
  static void BlinkSwitchLED(uint32_t r);
  static void off();
  static void on_up_to(uint8_t num);
  
  static constexpr uint8_t LED_num[LED_CHANNEL_LAST] = { LED1, LED2, LED3, LED4, LED5, LEDX };

private:
static uint32_t ticks_;
static uint32_t rate_;
static bool blink_;
};

}; // namespace 2CM

#endif // C2M_LED_H_
