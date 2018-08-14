#ifndef C2M_DIGITAL_INPUTS_H_
#define C2M_DIGITAL_INPUTS_H_

#include <stdint.h>
#include "C2M_config.h"
#include "C2M_core.h"
#include "C2M_gpio.h"

namespace C2M {

enum DigitalInput {
  DIGITAL_INPUT_1,
  DIGITAL_INPUT_2,
  DIGITAL_INPUT_3,
  DIGITAL_INPUT_4,
  DIGITAL_INPUT_5,
  DIGITAL_INPUT_LAST
};

#define DIGITAL_INPUT_MASK(x) (0x1 << (x))

static constexpr uint32_t DIGITAL_INPUT_1_MASK = DIGITAL_INPUT_MASK(DIGITAL_INPUT_1);
static constexpr uint32_t DIGITAL_INPUT_2_MASK = DIGITAL_INPUT_MASK(DIGITAL_INPUT_2);
static constexpr uint32_t DIGITAL_INPUT_3_MASK = DIGITAL_INPUT_MASK(DIGITAL_INPUT_3);
static constexpr uint32_t DIGITAL_INPUT_4_MASK = DIGITAL_INPUT_MASK(DIGITAL_INPUT_4);
static constexpr uint32_t DIGITAL_INPUT_5_MASK = DIGITAL_INPUT_MASK(DIGITAL_INPUT_5);

template <DigitalInput> struct InputPinDesc { };
template <> struct InputPinDesc<DIGITAL_INPUT_1> { static constexpr int PIN = TR1; };
template <> struct InputPinDesc<DIGITAL_INPUT_2> { static constexpr int PIN = TR2; };
template <> struct InputPinDesc<DIGITAL_INPUT_3> { static constexpr int PIN = TR3; };
template <> struct InputPinDesc<DIGITAL_INPUT_4> { static constexpr int PIN = TR4; };
template <> struct InputPinDesc<DIGITAL_INPUT_5> { static constexpr int PIN = TR5; };

class DigitalInputs {
public:

  static void Init();
  static void Scan();

  // @return mask of all pins cloked since last call, reset state
  static inline uint32_t clocked() {
    return clocked_mask_;
  }

  // @return mask if pin clocked since last call and reset state
  template <DigitalInput input> static inline uint32_t clocked() {
    return clocked_mask_ & (0x1 << input);
  }

  // @return mask if pin clocked since last call, reset state
  static inline uint32_t clocked(DigitalInput input) {
    return clocked_mask_ & (0x1 << input);
  }

  template <DigitalInput input> static inline bool read_immediate() {
    return !digitalReadFast(InputPinDesc<input>::PIN);
  }

  static inline bool read_immediate(DigitalInput input) {
    return !digitalReadFast(InputPinMap(input));
  }

  template <DigitalInput input> static inline void clock() {
    clocked_[input] = 1;
  }

private:

  inline static int InputPinMap(DigitalInput input) {
    switch (input) {
      case DIGITAL_INPUT_1: return InputPinDesc<DIGITAL_INPUT_1>::PIN;
      case DIGITAL_INPUT_2: return InputPinDesc<DIGITAL_INPUT_2>::PIN;
      case DIGITAL_INPUT_3: return InputPinDesc<DIGITAL_INPUT_3>::PIN;
      case DIGITAL_INPUT_4: return InputPinDesc<DIGITAL_INPUT_4>::PIN;
      case DIGITAL_INPUT_5: return InputPinDesc<DIGITAL_INPUT_5>::PIN;
      default: break;
    }
    return 0;
  }

  static uint32_t clocked_mask_;
  static volatile uint32_t clocked_[DIGITAL_INPUT_LAST];

  template <DigitalInput input>
  static uint32_t ScanInput() {
    if (clocked_[input]) {
      clocked_[input] = 0;
      return DIGITAL_INPUT_MASK(input);
    } else {
      return 0;
    }
  }
};

};

#endif // C2M_DIGITAL_INPUTS_H_
