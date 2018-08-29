#include "C2M_ADC.h"
#include "C2M_gpio.h"

#include <algorithm>

namespace C2M {

template <ADC_CHANNEL> struct ChannelDesc { };
template <> struct ChannelDesc<ADC_CHANNEL_1_1> {
  static const int PIN = CV1_1;
};
template <> struct ChannelDesc<ADC_CHANNEL_1_2> {
  static const int PIN = CV1_2;
};
template <> struct ChannelDesc<ADC_CHANNEL_2_1> {
  static const int PIN = CV2_1;
};
template <> struct ChannelDesc<ADC_CHANNEL_2_2> {
  static const int PIN = CV2_2;
};
template <> struct ChannelDesc<ADC_CHANNEL_3_1> {
  static const int PIN = CV3_1;
};
template <> struct ChannelDesc<ADC_CHANNEL_3_2> {
  static const int PIN = CV3_2;
};
template <> struct ChannelDesc<ADC_CHANNEL_4_1> {
  static const int PIN = CV4_1;
};
template <> struct ChannelDesc<ADC_CHANNEL_4_2> {
  static const int PIN = CV4_2;
};
template <> struct ChannelDesc<ADC_CHANNEL_5_1> {
  static const int PIN = CV5_1;
};
template <> struct ChannelDesc<ADC_CHANNEL_5_2> {
  static const int PIN = CV5_2;
};

/*static*/ ::ADC ADC::adc_;
/*static*/ size_t ADC::scan_channel_;
/*static*/ ADC::CalibrationData *ADC::calibration_data_;
/*static*/ uint32_t ADC::raw_[ADC_CHANNEL_LAST];
/*static*/ uint32_t ADC::smoothed_[ADC_CHANNEL_LAST];
#ifdef ENABLE_ADC_DEBUG
/*static*/ volatile uint32_t ADC::busy_waits_;
#endif

/*static*/ void ADC::Init(CalibrationData *calibration_data) {

  adc_.setReference(ADC_REF_3V3);
  adc_.setResolution(kAdcScanResolution);
  adc_.setConversionSpeed(kAdcConversionSpeed);
  adc_.setSamplingSpeed(kAdcSamplingSpeed);
  adc_.setAveraging(kAdcScanAverages);
  adc_.disableDMA();
  adc_.disableInterrupts();
  adc_.disableCompare();

  scan_channel_ = ADC_CHANNEL_1_1;
  adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_1_1>::PIN);

  calibration_data_ = calibration_data;
  std::fill(raw_, raw_ + ADC_CHANNEL_LAST, 0);
  std::fill(smoothed_, smoothed_ + ADC_CHANNEL_LAST, 0);
#ifdef ENABLE_ADC_DEBUG
  busy_waits_ = 0;
#endif
}

/*static*/ void FASTRUN ADC::Scan() {

#ifdef ENABLE_ADC_DEBUG
  if (!adc_.isComplete(ADC_0)) {
    ++busy_waits_;
    while (!adc_.isComplete(ADC_0));
  }
#endif
  const uint16_t value = adc_.readSingle(ADC_0);

  size_t channel = scan_channel_;

  // todo. basically, the 2_x inputs aren't as critical, so deal with them elsewhere / in some other fashion
  
  switch (channel) {
    case ADC_CHANNEL_1_1:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_2_1>::PIN, ADC_0);
      update<ADC_CHANNEL_1_1>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_2_1:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_3_1>::PIN, ADC_0);
      update<ADC_CHANNEL_2_1>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_3_1:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_4_1>::PIN, ADC_0);
      update<ADC_CHANNEL_3_1>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_4_1:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_5_1>::PIN, ADC_0);
      update<ADC_CHANNEL_4_1>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_5_1:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_1_2>::PIN, ADC_0);
      update<ADC_CHANNEL_5_1>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_1_2:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_2_2>::PIN, ADC_0);
      update<ADC_CHANNEL_1_2>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_2_2:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_3_2>::PIN, ADC_0);
      update<ADC_CHANNEL_2_2>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_3_2:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_4_2>::PIN, ADC_0);
      update<ADC_CHANNEL_3_2>(value);
      ++channel; 
      break;

   case ADC_CHANNEL_4_2:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_5_2>::PIN, ADC_0);
      update<ADC_CHANNEL_4_2>(value);
      ++channel; 
      break;

    case ADC_CHANNEL_5_2:
      adc_.startSingleRead(ChannelDesc<ADC_CHANNEL_1_1>::PIN, ADC_0);
      update<ADC_CHANNEL_5_2>(value);
      channel = ADC_CHANNEL_1_1;
      break;
  }
  scan_channel_ = channel;
}

/*static*/ void ADC::CalibratePitch(int32_t c2, int32_t c4) {
  // This is the method used by the Mutable Instruments calibration and
  // extrapolates from two octaves. 
  if (c2 < c4) {
    int32_t scale = (24 * 128 * 4096L) / (c4 - c2);
    calibration_data_->pitch_cv_scale = scale;
  }
}

}; // namespace C2M
