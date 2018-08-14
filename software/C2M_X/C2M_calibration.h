#ifndef C2M_CALIBRATION_H_
#define C2M_CALIBRATION_H_

#include "C2M_ADC.h"
#include "C2M_config.h"
#include "util/util_pagestorage.h"
#include "util/EEPROMStorage.h"

//#define VERBOSE_LUT
#ifdef VERBOSE_LUT
#define LUT_PRINTF(fmt, ...) serial_printf(fmt, ##__VA_ARGS__)
#else
#define LUT_PRINTF(x, ...) do {} while (0)
#endif

namespace C2M {

struct CalibrationData {
  static constexpr uint32_t FOURCC = FOURCC<'C', 'A', 'L', 1>::value;
  ADC::CalibrationData adc;
};

typedef PageStorage<EEPROMStorage, EEPROM_CALIBRATIONDATA_START, EEPROM_CALIBRATIONDATA_END, CalibrationData> CalibrationStorage;

extern CalibrationData calibration_data;

}; // namespace C2M

// Forward declarations for screwy build system
struct CalibrationStep;
struct CalibrationState;

#endif // C2M_CALIBRATION_H_
