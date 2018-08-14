#ifndef C2M_UI_H_
#define C2M_UI_H_

#include "C2M_config.h"
#include "UI/ui_button.h"
#include "UI/ui_event_queue.h"

namespace C2M {

struct App;

enum UiControl {
  CONTROL_BUTTON  = 0x1,
  CONTROL_BUTTON_MASK = 0xF,
  CONTROL_LAST = 0x2,
  CONTROL_BUTTON_LAST = 0x1,
};

static inline uint16_t control_mask(unsigned i) {
  return 1 << i;
}

enum UiMode {
  UI_MODE_MAIN,
  UI_MODE_SUSPEND,
  UI_MODE_CALIBRATE
};

class Ui {
public:
  static const size_t kEventQueueDepth = 16;
  static const uint32_t kLongPressTicks = 600;
  static const uint32_t kVeryLongPressTicks = 2500;
  
  Ui() { }

  void Init();

  UiMode calibration_mode();
  void Calibrate();
  void NukeSettings();
  void SaveSettings();
  UiMode DispatchEvents(C2M::App *app);

  void Poll();
  
  inline bool read_immediate(UiControl control) {
    return button_state_ & control;
  }
  
  inline uint32_t idle_time() const {
    return event_queue_.idle_time();
  }

  inline uint32_t ticks() const {
    return ticks_;
  }

  inline void SetButtonIgnoreMask() {
    button_ignore_mask_ = button_state_;
  }

  inline void IgnoreButton(UiControl control) {
    button_ignore_mask_ |= control;
  }

private:

  uint32_t ticks_;
  UI::Button buttons_[CONTROL_BUTTON_LAST];
  uint32_t button_press_time_[CONTROL_BUTTON_LAST];
  uint16_t button_state_;
  uint16_t button_ignore_mask_;

  UI::EventQueue<kEventQueueDepth> event_queue_;

  inline void PushEvent(UI::EventType t, uint16_t c, int16_t v, uint16_t m) {
    event_queue_.PushEvent(t, c, v, m);
  }

  bool IgnoreEvent(const UI::Event &event) {
    bool ignore = false;
    if (button_ignore_mask_ & event.control) {
      button_ignore_mask_ &= ~event.control;
      ignore = true;
    }
    return ignore;
  }
};

extern Ui ui;

}; // namespace C2M

#endif // C2M_UI_H_
