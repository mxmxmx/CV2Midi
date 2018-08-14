#ifndef CHANNEL_MESSAGE_QUANTIZER_H_
#define CHANNEL_MESSAGE_QUANTIZER_H_

class SemitoneQuantizer {
public:
  static constexpr int32_t kHysteresis = 16;

  SemitoneQuantizer() { }
  ~SemitoneQuantizer() { }

  void Init() {
    last_pitch_ = 0;
  }

  int32_t Process(int32_t pitch) {
    
    if ((pitch > last_pitch_ + kHysteresis) || (pitch < last_pitch_ - kHysteresis)) {
      last_pitch_ = pitch;
    } else {
      pitch = last_pitch_;
    }
    return (pitch + 63) >> 7;
  }

private:
  int32_t last_pitch_;
};
#endif
