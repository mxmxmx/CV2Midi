// Copyright (c) 2018 Max Stadler
//
// Author of app scaffolding: Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "util/util_settings.h"
#include "util/util_trigger_delay.h"
#include <MIDI.h>
#include "C2M_LED.h"
#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "channel_message_quantizer.h"
#include "extern/dspinst.h"

/* set default note number : */
#define DEFAULT_NOTE_NUM 60

enum CV2MIDISettings {
  CV2MIDI_SETTING_MIDI_CHANNEL,
  CV2MIDI_SETTING_DEFAULT_PITCH,
  CV2MIDI_SETTING_DEFAULT_VELOCITY,
  CV2MIDI_SETTING_MESSAGE_TYPE,
  CV2MIDI_SETTING_CC_NUMBER,
  CV2MIDI_SETTING_TRIGGER_DELAY,
  CV2MIDI_SETTING_LAST
};

enum ControlBitMask {
  CONTROL_GATE = 1,
  CONTROL_GATE_RISING = 2,
  CONTROL_GATE_FALLING = 4
};

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
const int32_t CM_SCALE = 71000; // scale factor --> velocity (~ x 1.05)
const int32_t NUM_CHANNELS = 5;
const int32_t DOUBLE_CLICK_TICKS = 2500;
const uint8_t trigger_delay_ticks[NUM_CHANNELS + 0x1] = { 0, 4, 8, 12, 16, 24 };

class CV2MIDI : public settings::SettingsBase<CV2MIDI, CV2MIDI_SETTING_LAST> {
public:

  void Init(C2M::DigitalInput default_trigger);

  C2M::DigitalInput get_trigger_input() const {
    return static_cast<C2M::DigitalInput>(channel_id_);
  }

  uint8_t get_midi_channel() const {
    return values_[CV2MIDI_SETTING_MIDI_CHANNEL];
  }

  void set_midi_channel(int8_t channel_num) {
    apply_value(CV2MIDI_SETTING_MIDI_CHANNEL, channel_num);
  }
  
  uint8_t get_default_pitch() const {
    return values_[CV2MIDI_SETTING_DEFAULT_PITCH];
  }

  void set_default_pitch(int8_t note_num) {
    apply_value(CV2MIDI_SETTING_DEFAULT_PITCH, note_num);
  }

  uint8_t get_default_velocity() const {
    return values_[CV2MIDI_SETTING_DEFAULT_VELOCITY];
  }

  void set_default_velocity(int8_t v) {
    apply_value(CV2MIDI_SETTING_DEFAULT_VELOCITY, v);
  }

  uint8_t get_channel_message() const {
    return values_[CV2MIDI_SETTING_MESSAGE_TYPE];
  }

  void set_channel_message(uint8_t message_type) {
    apply_value(CV2MIDI_SETTING_MESSAGE_TYPE, message_type);
  }
  
  uint16_t get_cc_number() const {
    return values_[CV2MIDI_SETTING_CC_NUMBER];
  }
  
  void set_cc_number(uint16_t cc_num) {
    apply_value(CV2MIDI_SETTING_CC_NUMBER, cc_num);
  }
  
  uint16_t get_trigger_delay() const {
    return values_[CV2MIDI_SETTING_TRIGGER_DELAY];
  }

  void set_trigger_delay(uint8_t delay_value) {
    apply_value(CV2MIDI_SETTING_TRIGGER_DELAY, delay_value);
  }

  int num_enabled_settings() const {
    return num_enabled_settings_;
  }

  CV2MIDISettings enabled_setting_at(int index) const {
    return enabled_settings_[index];
  }
  
  void update_enabled_settings() {
    CV2MIDISettings *settings = enabled_settings_;
    *settings++ = CV2MIDI_SETTING_MIDI_CHANNEL;
    *settings++ = CV2MIDI_SETTING_DEFAULT_PITCH;
    *settings++ = CV2MIDI_SETTING_DEFAULT_VELOCITY;
    *settings++ = CV2MIDI_SETTING_MESSAGE_TYPE;
    *settings++ = CV2MIDI_SETTING_CC_NUMBER;
    *settings++ = CV2MIDI_SETTING_TRIGGER_DELAY;
    num_enabled_settings_ = settings - enabled_settings_;
  }
  
  template <ADC_CHANNEL adc_channel>
  void FASTRUN Update(uint32_t triggers) {

    C2M::DigitalInput trigger_input = get_trigger_input();
    bool _gate_raised = C2M::DigitalInputs::read_immediate(trigger_input);
    bool _triggered = false;
    uint8_t _gate_state = 0;
      
    if (triggers & DIGITAL_INPUT_MASK(trigger_input)) {
      _gate_state |= CONTROL_GATE_RISING;
      _triggered = true;
    }

    trigger_delay_.Update();
    
    if (_triggered) {
      trigger_delay_.Push(trigger_delay_ticks[get_trigger_delay()]);
    }
    _triggered = trigger_delay_.triggered();
    
    if (_gate_raised)
      _gate_state |= CONTROL_GATE;
    else if (gate_raised_)
      _gate_state |= CONTROL_GATE_FALLING;
    gate_raised_ = _gate_raised;

    if (_triggered && (active_note_ < 0)) {

      // make note value:
      int32_t sample = C2M::ADC::pitch_value(adc_channel);
      sample = (quantizer_.Process(sample) >> 7) + get_default_pitch();
      CONSTRAIN(sample, 0, 127); 
      
      // ... and velocity (or CC value); scale to 0-127:
      int32_t cm_sample = Scale127(C2M::ADC::pitch_value(static_cast<ADC_CHANNEL>(adc_channel + ADC_CHANNEL_NUM)));

      switch (get_channel_message()) {
        case 0x90: // note on + velocity
        break;
        case 0xB0: // control change
          MIDI.sendControlChange(get_cc_number(), cm_sample, get_midi_channel());
          cm_sample = get_default_velocity(); // velocity
        break;
        /*
        case 0xA0: // aftertouch ... (probably should be sent later, not here / on trigger)
          MIDI.sendAfterTouch(cm_sample, get_midi_channel());
          cm_sample = get_default_velocity(); // velocity
        break;
        */
        default: break;
      }
      // send note on
      if (cm_sample) {
        MIDI.sendNoteOn(sample, cm_sample, get_midi_channel());
        active_note_ = sample;
        digitalWriteFast(C2M::LEDs::LED_num[channel_id_], HIGH);
      }
    }
    
    /* note off: once gate goes low, or is low ... */
    else if (((_gate_state == CONTROL_GATE_FALLING) || !_gate_raised) && (active_note_ >= 0)) {
      MIDI.sendNoteOn(active_note_, 0x0, get_midi_channel());
      /* alternative would be to use sendNoteOff ... */
      /* MIDI.sendNoteOff(active_note_, 0x0, _midi_channel);               */
      digitalWriteFast(C2M::LEDs::LED_num[channel_id_], LOW);
      active_note_ = -0xFF;
      gate_raised_ = false;
    }
  }

int32_t Scale127(int32_t sample) {
  
   int32_t sample_ = sample;
   sample_ = signed_multiply_32x16b(CM_SCALE, sample_);
   sample_ = signed_saturate_rshift(sample_, 16, 0);
   sample_ = cm_quantizer_.Process(sample_);
   CONSTRAIN(sample_, 0, 127);
   return sample_;
}

private:
  bool gate_raised_;
  uint8_t channel_id_;
  int32_t active_note_;
  braids::Quantizer quantizer_;
  SemitoneQuantizer cm_quantizer_;
  util::TriggerDelay<C2M::kMaxTriggerDelayTicks> trigger_delay_;
  int num_enabled_settings_;
  CV2MIDISettings enabled_settings_[CV2MIDI_SETTING_LAST];
};

void CV2MIDI::Init(C2M::DigitalInput default_trigger) {
  
  InitDefaults();
  channel_id_ = static_cast<int8_t>(default_trigger);
  // default to midi channels 1-5 ... only do this if defaults were not loaded in the first place:
  if (C2M::apps::using_defaults) set_midi_channel(channel_id_ + 0x1);
  active_note_ = -0xFF;
  gate_raised_ = false;
  quantizer_.Init();
  cm_quantizer_.Init();
  quantizer_.Configure(braids::scales[0x1], 0xFFFF);
  trigger_delay_.Init();
  update_enabled_settings();
}
  
SETTINGS_DECLARE(CV2MIDI, CV2MIDI_SETTING_LAST) {
  { 0x0, 0x0, 0xF, "midi ch ", NULL, settings::STORAGE_TYPE_U4 },
  { DEFAULT_NOTE_NUM, 0, 127, "default pitch", NULL, settings::STORAGE_TYPE_U8 },
  { DEFAULT_NOTE_NUM, 0, 127, "default velocity", NULL, settings::STORAGE_TYPE_U8 }, // used when sending CC or aftertouch
  { 0x90, 0x0, 0xFF, "message type", NULL, settings::STORAGE_TYPE_U8 },
  { 0, 0, 127, "cc number", NULL, settings::STORAGE_TYPE_U8 },
  { 2, 0, NUM_CHANNELS, "trigger delay", NULL, settings::STORAGE_TYPE_U4 } 
};

class C2M_CHANNEL {
public:
 
  void Init() {
    int input = C2M::DIGITAL_INPUT_1;
    for (auto &c2m : c2m_) {
      c2m.Init(static_cast<C2M::DigitalInput>(input));
      ++input;
    }
    ui.edit_mode = MODE_C2M;
    ui.selected_channel = 0x0;
    ui.trigger_settings = c2m_[0].get_trigger_delay();
    ui.arm = false;
    ui.save = false;
    ui.ticks = 0x0;
    ui.rx_status = 0x0;
    ui.pressed = 0x0;
    MIDI.begin(MIDI_CHANNEL_OMNI);
  }

  uint8_t delay_settings() { return c2m_[0].get_trigger_delay(); } 

  void reInit() { 
    for (int i = 0; i < NUM_CHANNELS; i++) 
    {
      c2m_[i].set_midi_channel(0x1 + i);
      c2m_[i].set_default_pitch(DEFAULT_NOTE_NUM);
      c2m_[i].set_trigger_delay(0x0); 
    }
  }
  
  void set_delay_settings(uint8_t val) { 
    // to do ... set per channel?
    for (int i = 0; i < NUM_CHANNELS; i++) 
      c2m_[i].set_trigger_delay(val);
  }

  void ISR() {
    
    uint32_t triggers = C2M::DigitalInputs::clocked();

    if (ui.edit_mode == C2M_CHANNEL::MODE_C2M) {
      c2m_[0].Update<ADC_CHANNEL_1_1>(triggers);
      c2m_[1].Update<ADC_CHANNEL_2_1>(triggers);
      c2m_[2].Update<ADC_CHANNEL_3_1>(triggers);
      c2m_[3].Update<ADC_CHANNEL_4_1>(triggers);
      c2m_[4].Update<ADC_CHANNEL_5_1>(triggers);  
    }
    else if (ui.edit_mode == C2M_CHANNEL::MODE_LEARN) {
      learn();
    }
    else if (ui.edit_mode == C2M_CHANNEL::MODE_SET_TR_DELAY) {
      update_trigger_delay();
    }
  }

  void learn() {
    
    ui.ticks++;
    int channel = ui.selected_channel;
    bool channel_armed = ui.arm;
    
    if (ui.pressed && ui.ticks > DOUBLE_CLICK_TICKS) {

        // advance channel
        ui.pressed = false;
        uint8_t prev_channel = ui.selected_channel;  
        ui.selected_channel++;
        if (ui.selected_channel >= NUM_CHANNELS) 
          ui.selected_channel = 0x0;
        SERIAL_PRINTLN("selected: channel %u, armed: %u", ui.selected_channel, ui.arm);
        digitalWriteFast(C2M::LEDs::LED_num[prev_channel], LOW);
        digitalWriteFast(C2M::LEDs::LED_num[ui.selected_channel], HIGH);
    }
    else if (channel_armed) {

      // blink LED
      if (ui.ticks < 3500)
        digitalWriteFast(C2M::LEDs::LED_num[ui.selected_channel], LOW);
      else if (ui.ticks < 7500)
        digitalWriteFast(C2M::LEDs::LED_num[ui.selected_channel], HIGH);
      else 
        ui.ticks = 0x0;

      /* and listen to incoming midi data: */
      
      if (Serial1.available() > 0) {

        uint8_t incomingByte = Serial1.read();
        uint8_t statusByte = incomingByte & 0xF0;
        
        switch(statusByte) {

          case 0x90: // note on
          {
            uint8_t chan_num = incomingByte & 0xF;
            uint8_t note_num = Serial1.read();
            uint8_t velocity = Serial1.read();
            
            /* set default pitch + midi channel, if velocity > 0 */
            if (velocity) {

              c2m_[channel].set_midi_channel(chan_num);
              c2m_[channel].set_channel_message(0x90); // set status
              c2m_[channel].set_default_pitch(note_num);
              SERIAL_PRINTLN("received: channel %d, note on: %d", c2m_[channel].get_midi_channel(), c2m_[channel].get_default_pitch());
              ui.save = true;
              ui.rx_status = millis();
              digitalWriteFast(LEDX, LOW);
            }
       
            /* echo the note */
            Serial1.write(incomingByte); Serial1.write(note_num); Serial1.write(velocity);
          }
          break;
          
          case 0xB0: // control change
          {

            uint8_t cc_num = Serial1.read();
            uint8_t cc_val = Serial1.read();
            
            /* set channel, status, and cc number: */
            c2m_[channel].set_midi_channel(incomingByte & 0xF);
            c2m_[channel].set_channel_message(0xB0);
            c2m_[channel].set_cc_number(cc_num);
            
            /* set default velocity via pot */
            int32_t velocity = c2m_[channel].Scale127(C2M::ADC::pitch_value(static_cast<ADC_CHANNEL>(channel + ADC_CHANNEL_NUM)));
            c2m_[channel].set_default_velocity(velocity);
            SERIAL_PRINTLN("received: channel %d, CC: %d / %d", c2m_[channel].get_midi_channel(), c2m_[channel].get_cc_number(),  cc_val);
            ui.save = true;
            ui.rx_status = millis();
            digitalWriteFast(LEDX, LOW);
            /* echo the note */
            Serial1.write(incomingByte); Serial1.write(cc_num); Serial1.write(cc_val);
          }
          break;
          //
          default:
          //SERIAL_PRINTLN("(ignored): channel %d, type: %d", MIDI.getChannel(), MIDI.getType());
          break;
        }
      }
      /* RX feedback ... */
      if (ui.rx_status && (millis() - ui.rx_status > 250)) {
        digitalWriteFast(LEDX, HIGH);
        ui.rx_status = 0x0;
      }
    }
  }

  void update_trigger_delay() {
    C2M::LEDs::Update();
  }

  enum Mode {
    MODE_C2M,
    MODE_LEARN,
    MODE_SET_TR_DELAY
  };

  struct {
    Mode edit_mode;
    int8_t selected_channel;
    int8_t trigger_settings;
    uint32_t ticks;
    uint32_t rx_status;
    bool arm;
    bool save;
    bool pressed;
  } ui;

  CV2MIDI c2m_[NUM_CHANNELS];
};

C2M_CHANNEL c2m_channels;

void CV2MIDI_init() {
  c2m_channels.Init();
}

size_t CV2MIDI_storageSize() {
  return NUM_CHANNELS * CV2MIDI::storageSize();
}

size_t CV2MIDI_save(void *storage) {
  size_t s = 0;
  for (auto &c2m : c2m_channels.c2m_)
    s += c2m.Save(static_cast<byte *>(storage) + s);
  return s;
}

size_t CV2MIDI_restore(const void *storage) {
  size_t s = 0;
  for (auto &c2m : c2m_channels.c2m_) {
    s += c2m.Restore(static_cast<const byte *>(storage) + s);
    c2m.update_enabled_settings();
  }
  return s;
}

void CV2MIDI_handleAppEvent(C2M::AppEvent event) {
  switch (event) {
    case C2M::APP_EVENT_RESUME:
      break;
    case C2M::APP_EVENT_SUSPEND:
      break;
  }
}

void CV2MIDI_button() {

  switch(c2m_channels.ui.edit_mode) {
    
    case C2M_CHANNEL::MODE_C2M:
    {
      c2m_channels.ui.edit_mode = C2M_CHANNEL::MODE_LEARN;
      c2m_channels.ui.selected_channel = 0x0;
      c2m_channels.ui.ticks = 0x0;
      c2m_channels.ui.arm = false;
      c2m_channels.ui.save = false;
      c2m_channels.ui.pressed = false;
      
      SERIAL_PRINTLN("LEARN ... ");
      // turn off lights:
      C2M::LEDs::off();
      // show channel
      digitalWriteFast(C2M::LEDs::LED_num[c2m_channels.ui.selected_channel], HIGH);
      // turn on switch LED: 
      digitalWriteFast(LEDX, HIGH);
    }
    break;
    case C2M_CHANNEL::MODE_LEARN:
    {  
      // double-click hack: 
      if (!c2m_channels.ui.arm) {
        if (c2m_channels.ui.ticks > (DOUBLE_CLICK_TICKS + 0xF)) {
          c2m_channels.ui.pressed = true;
        }
        else if (c2m_channels.ui.pressed && c2m_channels.ui.ticks < DOUBLE_CLICK_TICKS) {
          // 
          c2m_channels.ui.arm = true;
          c2m_channels.ui.pressed = false;
          SERIAL_PRINTLN("arming channel: %u", c2m_channels.ui.selected_channel);
        }        
      }
      else if (c2m_channels.ui.arm) {
          c2m_channels.ui.arm = false;
          digitalWriteFast(C2M::LEDs::LED_num[c2m_channels.ui.selected_channel], HIGH);
      }
      c2m_channels.ui.ticks = 0x0;
    }
    break;
    case C2M_CHANNEL::MODE_SET_TR_DELAY:
    {
     // choose trigger delay settings:
     if (c2m_channels.ui.trigger_settings++ >= NUM_CHANNELS)
      c2m_channels.ui.trigger_settings = 0x0;
     SERIAL_PRINTLN("trigger delay setting: %u", c2m_channels.ui.trigger_settings);
     // show settings > 0
     C2M::LEDs::on_up_to(c2m_channels.ui.trigger_settings);
     c2m_channels.set_delay_settings(c2m_channels.ui.trigger_settings);
     c2m_channels.ui.save = true;
    }
    break;
    default: break;
  }
}

void CV2MIDI_button_long() {

  switch(c2m_channels.ui.edit_mode) {
    
    case C2M_CHANNEL::MODE_C2M:
    {
      c2m_channels.ui.edit_mode = C2M_CHANNEL::MODE_SET_TR_DELAY;
      // turn off lights:
      C2M::LEDs::off();
      C2M::LEDs::BlinkSwitchLED(5000);
      c2m_channels.ui.trigger_settings = c2m_channels.delay_settings();
      // show settings > 0
      C2M::LEDs::on_up_to(c2m_channels.ui.trigger_settings);
    }
    break;
    case C2M_CHANNEL::MODE_LEARN:
    {
      if (!c2m_channels.ui.arm) {
        // exit + save
        if (c2m_channels.ui.save)
          C2M::ui.SaveSettings();
        c2m_channels.ui.save = false;  
        c2m_channels.ui.selected_channel = 0x0;
        c2m_channels.ui.edit_mode = C2M_CHANNEL::MODE_C2M;
        C2M::LEDs::off(); 
      }
    }
    break;
    case C2M_CHANNEL::MODE_SET_TR_DELAY:
    {
      // exit + save
      if (c2m_channels.ui.save)
        C2M::ui.SaveSettings();
      c2m_channels.ui.save = false;
      c2m_channels.ui.edit_mode = C2M_CHANNEL::MODE_C2M;
      C2M::LEDs::off();
    }
    break;
    default: break;
  }
}

void CV2MIDI_button_very_long() {
  
  // reset to defaults
  if (c2m_channels.ui.edit_mode == C2M_CHANNEL::MODE_C2M) {

    C2M::LEDs::on_up_to(NUM_CHANNELS);
    SERIAL_PRINTLN("Nuke settings ... ");
    C2M::ui.NukeSettings();
    c2m_channels.reInit();
    C2M::LEDs::off();
  }
}

void CV2MIDI_loop() {
}

void CV2MIDI_handleButtonEvent(const UI::Event &event) {
  
  switch (event.type) {
    case UI::EVENT_BUTTON_PRESS:
      CV2MIDI_button();
    break;
    case UI::EVENT_BUTTON_LONG_PRESS:
      CV2MIDI_button_long();
    break;
    case UI::EVENT_BUTTON_VERY_LONG_PRESS:
      CV2MIDI_button_very_long();
    break;
    default: break;  
  }
}

void FASTRUN CV2MIDI_isr() {
  c2m_channels.ISR();
}
// end ... 
