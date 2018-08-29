// Copyright (c) 2015-2018 Max Stadler, Patrick Dowling
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

// everything based on o_C ...

#include <EEPROM.h>
#include "C2M_apps.h"
#include "C2M_core.h"
#include "C2M_LED.h"
#include "C2M_gpio.h"
#include "C2M_ADC.h"
#include "C2M_calibration.h"
#include "C2M_digital_inputs.h"
#include "C2M_ui.h"
#include "C2M_version.h"
#include "C2M_options.h"
#include "src/drivers/ADC/C2M_util_ADC.h"
#include "util/util_debugpins.h"

C2M::UiMode ui_mode = C2M::UI_MODE_MAIN;
extern void calibration_load();

/*  ------------------------ UI timer ISR ---------------------------   */

IntervalTimer UI_timer;

void FASTRUN UI_timer_ISR() {
  C2M::ui.Poll();
}

/*  ------------------------ core timer ISR ---------------------------   */
IntervalTimer CORE_timer;
volatile bool C2M::CORE::app_isr_enabled = false;
volatile uint32_t C2M::CORE::ticks = 0;

void FASTRUN CORE_timer_ISR() {

  // we sequence the ADC pins using DMA. 
  // with kAdcScanAverages = 4 and kAdcScanResolution = 16, there's a new set of readings available every ~ 7.5kHz
  C2M::ADC::Scan_DMA();

  // Pin changes are tracked in separate ISRs, so depending on prio it might
  // need extra precautions.
  C2M::DigitalInputs::Scan();

  ++C2M::CORE::ticks;
  
  if (C2M::CORE::app_isr_enabled)
    C2M::apps::ISR();
}

/*       ---------------------------------------------------------         */

void setup() {
  
  delay(250);

  SERIAL_PRINTLN("* CV / MIDI BOOTING...");
  SERIAL_PRINTLN("* %s", C2M_VERSION);

  C2M::DigitalInputs::Init();
  C2M::ADC::Init(&C2M::calibration_data.adc);
  C2M::ADC::Init_DMA();
  C2M::LEDs::Init(); 
  C2M::ui.Init();
  calibration_load();

  SERIAL_PRINTLN("* CORE ISR @%luus", C2M_CORE_TIMER_RATE);
  CORE_timer.begin(CORE_timer_ISR, C2M_CORE_TIMER_RATE);
  CORE_timer.priority(C2M_CORE_TIMER_PRIO);

  SERIAL_PRINTLN("* UI ISR @%luus", C2M_UI_TIMER_RATE);
  UI_timer.begin(UI_timer_ISR, C2M_UI_TIMER_RATE);
  UI_timer.priority(C2M_UI_TIMER_PRIO);

  // optional calibration
  ui_mode = C2M::ui.calibration_mode();

  if (ui_mode == C2M::UI_MODE_CALIBRATE) {
    C2M::ui.Calibrate();
    ui_mode = C2M::UI_MODE_MAIN;
  }
  // initialize app
  C2M::apps::Init();
}

/*  ---------    main loop  --------  */

void FASTRUN loop() {

  C2M::CORE::app_isr_enabled = true;
  
  while (true) {
    // Run current app
    C2M::apps::current_app->loop();
    // Ui
    C2M::ui.DispatchEvents(C2M::apps::current_app);
  }
}
