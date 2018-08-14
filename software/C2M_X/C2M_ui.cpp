#include <Arduino.h>
#include <algorithm>

#include "C2M_apps.h"
#include "C2M_calibration.h"
#include "C2M_config.h"
#include "C2M_core.h"
#include "C2M_gpio.h"
#include "C2M_ui.h"
#include "C2M_version.h"
#include "C2M_options.h"

namespace C2M {

Ui ui;

void Ui::Init() {
  
  ticks_ = 0;
  static const int button_pins[] = { SWITCH };
  for (size_t i = 0; i < CONTROL_BUTTON_LAST; ++i) {
    buttons_[i].Init(button_pins[i], C2M_GPIO_BUTTON_PINMODE);
  }
  std::fill(button_press_time_, button_press_time_ + 4, 0);
  button_state_ = 0;
  button_ignore_mask_ = 0;
  event_queue_.Init();
}

void FASTRUN Ui::Poll() {

  uint32_t now = ++ticks_;
  uint16_t button_state = 0;
 
  for (size_t i = 0; i < CONTROL_BUTTON_LAST; ++i) {
    if (buttons_[i].Poll())
      button_state |= control_mask(i);
  }

  for (size_t i = 0; i < CONTROL_BUTTON_LAST; ++i) {
    
    auto &button = buttons_[i];
    
    if (button.just_pressed()) {
      button_press_time_[i] = now;
    } 
    else if ((now - button_press_time_[i] == kVeryLongPressTicks + 0xF) && button.read_immediate()) {
      digitalWriteFast(LEDX, HIGH);
    }
    else if (button.released()) {
      if (now - button_press_time_[i] < kLongPressTicks) {
        PushEvent(UI::EVENT_BUTTON_PRESS, control_mask(i), 0, button_state);
      }
      else if (now - button_press_time_[i] < kVeryLongPressTicks) {
        PushEvent(UI::EVENT_BUTTON_LONG_PRESS, control_mask(i), 0, button_state);
      }
      else {
        PushEvent(UI::EVENT_BUTTON_VERY_LONG_PRESS, control_mask(i), 0, button_state);
          digitalWriteFast(LEDX, LOW);
      }
    }
  }

  button_state_ = button_state;
}

UiMode Ui::DispatchEvents(App *app) {

  while (event_queue_.available()) {
    const UI::Event event = event_queue_.PullEvent();
    if (IgnoreEvent(event))
      continue;

    switch (event.type) {
      case UI::EVENT_BUTTON_PRESS:
      case UI::EVENT_BUTTON_LONG_PRESS:
      case UI::EVENT_BUTTON_VERY_LONG_PRESS:
        app->HandleButtonEvent(event);
        break;
      default:
        break;
    }
  }
  return UI_MODE_MAIN;
}

UiMode Ui::calibration_mode() {

  UiMode mode = UI_MODE_MAIN;

  Poll();

  if (C2M::ui.read_immediate(CONTROL_BUTTON)) {
    // hack to avoid ... waking up in calibration mode
    delay(50);
    if (C2M::ui.read_immediate(CONTROL_BUTTON))
      mode = UI_MODE_CALIBRATE;
  }
    
  while (event_queue_.available())
      (void)event_queue_.PullEvent();
      
  SetButtonIgnoreMask();
  return mode;
}

} // namespace C2M
