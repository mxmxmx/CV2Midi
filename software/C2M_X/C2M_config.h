#ifndef C2M_CONFIG_H_
#define C2M_CONFIG_H_

#if F_CPU != 120000000
#error "Please compile firmware with CPU speed 120MHz"
#endif

static constexpr uint32_t C2M_CORE_ISR_FREQ = 15000U;
static constexpr uint32_t C2M_CORE_TIMER_RATE = (1000000UL / C2M_CORE_ISR_FREQ);
static constexpr uint32_t C2M_UI_TIMER_RATE   = 2000UL;

// From kinetis.h
// Cortex-M4: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240
static constexpr int C2M_CORE_TIMER_PRIO = 96;  // yet higher
static constexpr int C2M_GPIO_ISR_PRIO   = 112; // higher
static constexpr int C2M_UI_TIMER_PRIO   = 128; // default

namespace C2M {
static constexpr size_t kMaxTriggerDelayTicks = 96;
};

#define OCTAVES 10      // # octaves
#define SEMITONES (OCTAVES * 12)

static constexpr unsigned long APP_SELECTION_TIMEOUT_MS = 25000;
static constexpr unsigned long SETTINGS_SAVE_TIMEOUT_MS = 1000;

#define EEPROM_CALIBRATIONDATA_START 0
#define EEPROM_CALIBRATIONDATA_END 128

#define EEPROM_GLOBALSETTINGS_START EEPROM_CALIBRATIONDATA_END
#define EEPROM_GLOBALSETTINGS_END 850 

#define EEPROM_APPDATA_START EEPROM_GLOBALSETTINGS_END
#define EEPROM_APPDATA_END EEPROMStorage::LENGTH

// This is the available space for all apps' settings 
#define EEPROM_APPDATA_BINARY_SIZE (1000 - 4)

#define C2M_UI_DEBUG
#define C2M_UI_SEPARATE_ISR

#define C2M_CALIBRATION_DEFAULT_FLAGS (0)

#endif // C2M_CONFIG_H_
