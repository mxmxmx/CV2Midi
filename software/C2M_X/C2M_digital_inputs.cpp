#include <Arduino.h>
#include <algorithm>
#include "C2M_digital_inputs.h"
#include "C2M_gpio.h"
#include "C2M_options.h"

/*static*/
uint32_t C2M::DigitalInputs::clocked_mask_;

/*static*/
volatile uint32_t C2M::DigitalInputs::clocked_[DIGITAL_INPUT_LAST];

void FASTRUN tr1_ISR() {  
  C2M::DigitalInputs::clock<C2M::DIGITAL_INPUT_1>();
}  // main clock

void FASTRUN tr2_ISR() {
  C2M::DigitalInputs::clock<C2M::DIGITAL_INPUT_2>();
}

void FASTRUN tr3_ISR() {
  C2M::DigitalInputs::clock<C2M::DIGITAL_INPUT_3>();
}

void FASTRUN tr4_ISR() {
  C2M::DigitalInputs::clock<C2M::DIGITAL_INPUT_4>();
}

void FASTRUN tr5_ISR() {
  C2M::DigitalInputs::clock<C2M::DIGITAL_INPUT_5>();
}

/*static*/
void C2M::DigitalInputs::Init() {

  static const struct {
    uint8_t pin;
    void (*isr_fn)();
  } pins[DIGITAL_INPUT_LAST] =  {
    {TR1, tr1_ISR},
    {TR2, tr2_ISR},
    {TR3, tr3_ISR},
    {TR4, tr4_ISR},
    {TR5, tr5_ISR}
  };

  for (auto pin : pins) {
    pinMode(pin.pin, C2M_GPIO_TRx_PINMODE);
    attachInterrupt(pin.pin, pin.isr_fn, FALLING);
  }

  clocked_mask_ = 0;
  std::fill(clocked_, clocked_ + DIGITAL_INPUT_LAST, 0);

}

/*static*/
void C2M::DigitalInputs::Scan() {
  clocked_mask_ =
    ScanInput<DIGITAL_INPUT_1>() |
    ScanInput<DIGITAL_INPUT_2>() |
    ScanInput<DIGITAL_INPUT_3>() |
    ScanInput<DIGITAL_INPUT_4>() |
    ScanInput<DIGITAL_INPUT_5>(); 
}
