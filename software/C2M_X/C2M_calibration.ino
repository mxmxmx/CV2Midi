#include "C2M_calibration.h"
#include "C2M_LED.h"

static constexpr uint16_t _ADC_OFFSET = (uint16_t)((float)pow(2,C2M::ADC::kAdcResolution)*0.5f); // ADC offset @1.65V
static constexpr uint16_t _ADC_ZERO_OFFSET = (uint16_t)((float)pow(2,C2M::ADC::kAdcResolution)*1.0f); // ADC offset @3.30V

namespace C2M {
  
CalibrationStorage calibration_storage;
CalibrationData calibration_data;

};

static constexpr unsigned kCalibrationAdcSmoothing = 4;
bool calibration_data_loaded = false;

const C2M::CalibrationData kCalibrationDefaults = {
  
  // ADC
  { 
    { _ADC_OFFSET, _ADC_OFFSET, _ADC_OFFSET, _ADC_OFFSET, _ADC_OFFSET, _ADC_ZERO_OFFSET, _ADC_ZERO_OFFSET, _ADC_ZERO_OFFSET, _ADC_ZERO_OFFSET, _ADC_ZERO_OFFSET },
    0  // pitch_cv_scale
  }
};

void calibration_reset() {
  memcpy(&C2M::calibration_data, &kCalibrationDefaults, sizeof(C2M::calibration_data));
}

void calibration_load() {
  SERIAL_PRINTLN("Cal.Storage: PAGESIZE=%u, PAGES=%u, LENGTH=%u",
                 C2M::CalibrationStorage::PAGESIZE, C2M::CalibrationStorage::PAGES, C2M::CalibrationStorage::LENGTH);

  calibration_reset();
  calibration_data_loaded = C2M::calibration_storage.Load(C2M::calibration_data);
  if (!calibration_data_loaded) {
    SERIAL_PRINTLN("No calibration data, using defaults");
  } else {
    SERIAL_PRINTLN("Calibration data loaded...");
  }

  if (!C2M::calibration_data.adc.pitch_cv_scale) {
    SERIAL_PRINTLN("CV scale not set, using default");
    C2M::calibration_data.adc.pitch_cv_scale = C2M::ADC::kDefaultPitchCVScale;
  }
}

void calibration_save() {
  SERIAL_PRINTLN("Saving calibration data");
  C2M::calibration_storage.Save(C2M::calibration_data);
}

enum CALIBRATION_STEP {  
  HELLO,
  CV_OFFSET,
  ADC_PITCH_C2, 
  ADC_PITCH_C4,
  CALIBRATION_SAVE,
  CALIBRATION_STEP_LAST
};  

enum CALIBRATION_TYPE {
  CALIBRATE_NONE,
  CALIBRATE_ADC_OFFSET,
  CALIBRATE_ADC_1V,
  CALIBRATE_ADC_3V,
  CALIBRATE_SAVE
};

struct CalibrationStep {
  CALIBRATION_STEP step;
  CALIBRATION_TYPE calibration_type;
};

struct CalibrationState {
  
  CALIBRATION_STEP step;
  const CalibrationStep *current_step;
  uint16_t adc_1v;
  uint16_t adc_3v;
  bool used_defaults;
};


const CalibrationStep calibration_steps[CALIBRATION_STEP_LAST] = 
{  
  { HELLO, CALIBRATE_NONE },
  { CV_OFFSET, CALIBRATE_ADC_OFFSET },
  { ADC_PITCH_C2, CALIBRATE_ADC_1V },
  { ADC_PITCH_C4, CALIBRATE_ADC_3V },
  { CALIBRATION_SAVE, CALIBRATE_SAVE }
};

/*     loop calibration menu until done       */
void C2M::Ui::Calibrate() {

  // Calibration data should be loaded (or defaults) by now
  SERIAL_PRINTLN("Start calibration...");
  C2M::LEDs::BlinkSwitchLED(1000);

  CalibrationState calibration_state = {
    HELLO,
    &calibration_steps[HELLO]
  };
  
  calibration_state.used_defaults = false;
  bool calibration_complete = false;
  
  while (!calibration_complete) {

    while (event_queue_.available()) {
      
      const UI::Event event = event_queue_.PullEvent();
      
      if (IgnoreEvent(event))
        continue;

      switch (event.control) {
        
        case CONTROL_BUTTON:
          if (calibration_state.step == HELLO && UI::EVENT_BUTTON_LONG_PRESS == event.type) {
            calibration_reset();
            SERIAL_PRINTLN("reset to defaults");
            digitalWriteFast(LEDX, HIGH);
            delay(100);
            digitalWriteFast(LEDX, LOW);
            delay(100);
            digitalWriteFast(LEDX, HIGH);
            delay(100);
            digitalWriteFast(LEDX, LOW);
            delay(100);
            digitalWriteFast(LEDX, HIGH);
            delay(100);
            digitalWriteFast(LEDX, LOW);
            delay(100);
          }
          else if (calibration_state.step < CALIBRATION_STEP_LAST && UI::EVENT_BUTTON_PRESS == event.type) {
            calibration_state.step = static_cast<CALIBRATION_STEP>(calibration_state.step + 1);
            SERIAL_PRINTLN("next step -> %d", calibration_state.step);
          }
          break;
        default:
          break;
      }
    }

    const CalibrationStep *next_step = &calibration_steps[calibration_state.step];
    
    if (next_step != calibration_state.current_step) {
     
      // Special cases on exit current step
      switch (calibration_state.current_step->step) {
        case HELLO:
          digitalWriteFast(LEDX, LOW);
          break;
        case CV_OFFSET:
        // turn of lights again
        for (int i = 0; i < ADC_CHANNEL_NUM; i++)
          digitalWriteFast(C2M::LEDs::LED_num[i], LOW);
        digitalWriteFast(C2M::LEDs::LED_num[0], HIGH); 
        break;
        case ADC_PITCH_C2:
        // turn on 3 lights --> 3V
        for (int i = 0; i < 3; i++)
          digitalWriteFast(C2M::LEDs::LED_num[i], HIGH);
        break;
        case ADC_PITCH_C4:
          if (calibration_state.adc_1v && calibration_state.adc_3v) {
            // using the raw values, ie C4 < C2 
            C2M::ADC::CalibratePitch(calibration_state.adc_3v, calibration_state.adc_1v);
            SERIAL_PRINTLN("ADC SCALE 1V=%d, 3V=%d -> %d",
                           calibration_state.adc_1v, calibration_state.adc_3v,
                           C2M::calibration_data.adc.pitch_cv_scale);
          }// turn off again 3 lights 
          for (int i = 0; i < 3; i++)
            digitalWriteFast(C2M::LEDs::LED_num[i], LOW);
          digitalWriteFast(LEDX, HIGH);
          // now save calibration data
          SERIAL_PRINTLN("Calibration complete");
          calibration_save();
          calibration_complete = true;
          digitalWriteFast(LEDX, LOW);
          delay(100);
          digitalWriteFast(LEDX, HIGH);
          delay(100);
          digitalWriteFast(LEDX, LOW);
          delay(100);
          digitalWriteFast(LEDX, HIGH);
          delay(100);
          digitalWriteFast(LEDX, LOW);
          delay(100);
          digitalWriteFast(LEDX, HIGH);
          delay(100);
          digitalWriteFast(LEDX, LOW);
          C2M::LEDs::BlinkSwitchLED(0x0);
          break;
        default: break;
      }

      // Setup next step
      switch (next_step->calibration_type) {
      case CALIBRATE_ADC_OFFSET:
        for (int i = 0; i < ADC_CHANNEL_NUM; i++) {
        C2M::calibration_data.adc.offset[i] = adc_average(static_cast<ADC_CHANNEL>(i)); 
        C2M::calibration_data.adc.offset[i + ADC_CHANNEL_NUM] = adc_average(static_cast<ADC_CHANNEL>(i + ADC_CHANNEL_NUM));
        digitalWriteFast(C2M::LEDs::LED_num[i], HIGH);
        } 
        break;
      case CALIBRATE_ADC_1V:
        calibration_state.adc_1v = adc_average(ADC_CHANNEL_1_1);
        break;
      case CALIBRATE_ADC_3V:
        calibration_state.adc_3v = adc_average(ADC_CHANNEL_1_1);
        break;
      default:
        break;
      }
      calibration_state.current_step = next_step;
    }
    calibration_update(calibration_state);
  }
}

void calibration_update(CalibrationState &state) {

  const CalibrationStep *step = state.current_step;

  C2M::LEDs::Update();
  
  switch (step->calibration_type) {

    case HELLO:
   
      break;
    case CALIBRATE_ADC_OFFSET:
      break;
    default: break;
  }
}

/* misc */ 

uint32_t adc_average(ADC_CHANNEL channel) {

  uint32_t n = 0, val = 0;
  while (n++ < 16) {
    delay(5);
    val += C2M::ADC::smoothed_raw_value(channel);
  }
  SERIAL_PRINTLN("average value (channel %d): %d ", channel, (int)(val >> 4));
  return val >> 4;
}

