#ifndef C2M_MIDI_H_
#define C2M_MIDI_H_

#include <stdint.h>

namespace C2M {

class MIDI {
  
public:

  static void Init();
  static int rx();
  static void send_data(uint8_t statusByte, uint8_t param1, uint8_t param2);
 
};

}; // namespace 2CM

#endif // C2M_MIDI_H_
