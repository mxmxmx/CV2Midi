#ifndef C2M_CORE_H_
#define C2M_CORE_H_

#include <stdint.h>
#include "C2M_config.h"
#include "util/util_debugpins.h"

namespace C2M {
  namespace CORE {
  extern volatile uint32_t ticks;
  extern volatile bool app_isr_enabled;

  }; // namespace CORE


  struct TickCount {
    TickCount() { }
    void Init() {
      last_ticks = 0;
    }

    uint32_t Update() {
      uint32_t now = C2M::CORE::ticks;
      uint32_t ticks = now - last_ticks;
      last_ticks = now;
      return ticks;
    }

    uint32_t last_ticks;
  };
}; // namespace C2M

#endif // C2M_CORE_H_
