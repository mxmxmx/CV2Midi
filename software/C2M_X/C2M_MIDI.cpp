#include "C2M_MIDI.h"
#include "Arduino.h"

 namespace C2M {

/*static*/
void MIDI::Init() {
  Serial1.begin(31250);
}

int MIDI::rx() {
  return (Serial1.available() > 0);
}

void MIDI::send_data(uint8_t statusByte, uint8_t param1, uint8_t param2) {
  Serial1.write(statusByte); Serial1.write(param1); Serial1.write(param2);
}

}; // namespace 2CM

// C2M_MIDI
