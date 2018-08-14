#include "C2M_gpio.h"
#include "C2M_LED.h"
#include "C2M_options.h"
#include "C2M_calibration.h"
 
namespace C2M {

/*static*/
void LEDs::Init() {

  ticks_ = 0x0;
  rate_ = 0x0;
  blink_ = 0x0;
  
  for (uint16_t i = 0; i < LED_CHANNEL_LAST; i++) {
    pinMode(LED_num[i], C2M_GPIO_LEDx_PINMODE);
  }
}

void LEDs::Update() {

  ticks_++;
  
  if (blink_) {
    if (ticks_ < (rate_ >> 1))
      digitalWriteFast(LEDX, LOW);
    else if (ticks_ < rate_)
      digitalWriteFast(LEDX, HIGH);
    else ticks_ = 0x0;
  }
  
}

void LEDs::BlinkSwitchLED(uint32_t r) {

  if (r) {
    blink_ = true;
    rate_ = r;
  }
  else blink_ = false;
}

void LEDs::off() { 
   for (int i = 0; i < LED_CHANNEL_LAST; i++)
   digitalWriteFast(LED_num[i], LOW);
}

void LEDs::on_up_to(uint8_t num) {

  if (num) {
    for (int i = 0; i < num; i++) {
      digitalWriteFast(LED_num[i], HIGH);
    }
  }
  else off();
}
/*static*/
uint8_t constexpr LEDs::LED_num[LED_CHANNEL_LAST];
uint32_t LEDs::ticks_;
uint32_t LEDs::rate_;
bool LEDs::blink_;


}; // namespace 2CM

// C2M_LED
