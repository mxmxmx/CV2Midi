// Copyright (c) 2016 Patrick Dowling
//
// Author: Patrick Dowling (pld@gurkenkiste.com)
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

#include "C2M_apps.h"
#include "C2M_digital_inputs.h"

#define DECLARE_APP(a, b, name, prefix, isr) \
{ TWOCC<a,b>::value, name, \
  prefix ## _init, prefix ## _storageSize, prefix ## _save, prefix ## _restore, \
  prefix ## _handleAppEvent, \
  prefix ## _loop, \
  prefix ## _handleButtonEvent, \
  isr \
}

C2M::App available_apps[] = {
  DECLARE_APP('C','M', "CV2MIDI", CV2MIDI, CV2MIDI_isr)
};

static constexpr int NUM_AVAILABLE_APPS = ARRAY_SIZE(available_apps);

namespace C2M {

// Global settings are stored separately to actual app setings.
// The theory is that they might not change as often.
struct GlobalSettings {
  static constexpr uint32_t FOURCC = FOURCC<'O','C','S',2>::value;
  bool reserved0;
  bool reserved1;
  uint16_t current_app_id;
};

// App settings are packed into a single blob of binary data; each app's chunk
// gets its own header with id and the length of the entire chunk. This makes
// this a bit more flexible during development.
// Chunks are aligned on 2-byte boundaries for arbitrary reasons (thankfully M4
// allows unaligned access...)
struct AppChunkHeader {
  uint16_t id;
  uint16_t length;
} __attribute__((packed));

struct AppData {
  static constexpr uint32_t FOURCC = FOURCC<'O','C','A',4>::value;
  static constexpr size_t kAppDataSize = EEPROM_APPDATA_BINARY_SIZE;
  char data[kAppDataSize];
  size_t used;
};

typedef PageStorage<EEPROMStorage, EEPROM_GLOBALSETTINGS_START, EEPROM_GLOBALSETTINGS_END, GlobalSettings> GlobalSettingsStorage;
typedef PageStorage<EEPROMStorage, EEPROM_APPDATA_START, EEPROM_APPDATA_END, AppData> AppDataStorage;

GlobalSettings global_settings;
GlobalSettingsStorage global_settings_storage;

AppData app_settings;
AppDataStorage app_data_storage;

static constexpr int DEFAULT_APP_INDEX = 0;
static const uint16_t DEFAULT_APP_ID = available_apps[DEFAULT_APP_INDEX].id;

void save_global_settings() {
  SERIAL_PRINTLN("Save global settings");
  global_settings_storage.Save(global_settings);
  SERIAL_PRINTLN("Saved global settings: page_index %d", global_settings_storage.page_index());
}

void save_app_data() {
  
  SERIAL_PRINTLN("Save app data... (%u bytes available)", C2M::AppData::kAppDataSize);

  app_settings.used = 0;
  char *data = app_settings.data;
  char *data_end = data + C2M::AppData::kAppDataSize;

  size_t start_app = random(NUM_AVAILABLE_APPS);
  for (size_t i = 0; i < NUM_AVAILABLE_APPS; ++i) {
    const auto &app = available_apps[(start_app + i) % NUM_AVAILABLE_APPS];
    size_t storage_size = app.storageSize() + sizeof(AppChunkHeader);
    if (storage_size & 1) ++storage_size; // Align chunks on 2-byte boundaries
    if (storage_size > sizeof(AppChunkHeader) && app.Save) {
      if (data + storage_size > data_end) {
        SERIAL_PRINTLN("%s: ERROR: %u BYTES NEEDED, %u BYTES AVAILABLE OF %u BYTES TOTAL", app.name, storage_size, data_end - data, AppData::kAppDataSize);
        continue;
      }

      AppChunkHeader *chunk = reinterpret_cast<AppChunkHeader *>(data);
      chunk->id = app.id;
      chunk->length = storage_size;
      #ifdef PRINT_DEBUG
        SERIAL_PRINTLN("* %s (%02x) : Saved %u bytes... (%u)", app.name, app.id, app.Save(chunk + 1), storage_size);
      #else
        app.Save(chunk + 1);
      #endif
      app_settings.used += chunk->length;
      data += chunk->length;
    }
  }
  SERIAL_PRINTLN("App settings used: %u/%u", app_settings.used, EEPROM_APPDATA_BINARY_SIZE);
  app_data_storage.Save(app_settings);
  SERIAL_PRINTLN("Saved app settings in page_index %d", app_data_storage.page_index());
}

void restore_app_data() {
  SERIAL_PRINTLN("Restoring app data from page_index %d, used=%u", app_data_storage.page_index(), app_settings.used);

  const char *data = app_settings.data;
  const char *data_end = data + app_settings.used;
  size_t restored_bytes = 0;

  while (data < data_end) {
    const AppChunkHeader *chunk = reinterpret_cast<const AppChunkHeader *>(data);
    if (data + chunk->length > data_end) {
      SERIAL_PRINTLN("App chunk length %u exceeds available space (%u)", chunk->length, data_end - data);
      break;
    }

    App *app = apps::find(chunk->id);
    if (!app) {
      SERIAL_PRINTLN("App %02x not found, ignoring chunk...", app->id);
      if (!chunk->length)
        break;
      data += chunk->length;
      continue;
    }
    size_t expected_length = app->storageSize() + sizeof(AppChunkHeader);
    if (expected_length & 0x1) ++expected_length;
    if (chunk->length != expected_length) {
      SERIAL_PRINTLN("* %s (%02x): chunk length %u != %u (storageSize=%u), skipping...", app->name, chunk->id, chunk->length, expected_length, app->storageSize());
      data += chunk->length;
      continue;
    }

    if (app->Restore) {
      #ifdef PRINT_DEBUG
        SERIAL_PRINTLN("* %s (%02x): Restored %u from %u (chunk length %u)...", app->name, chunk->id, app->Restore(chunk + 1), chunk->length - sizeof(AppChunkHeader), chunk->length);
      #else
        app->Restore(chunk + 1);
      #endif
    }
    restored_bytes += chunk->length;
    data += chunk->length;
  }

  SERIAL_PRINTLN("App data restored: %u, expected %u", restored_bytes, app_settings.used);
}

namespace apps {

void set_current_app(int index) {
  current_app = &available_apps[index];
  global_settings.current_app_id = current_app->id;
}

App *current_app = &available_apps[DEFAULT_APP_INDEX];

App *find(uint16_t id) {
  for (auto &app : available_apps)
    if (app.id == id) return &app;
  return nullptr;
}

bool using_defaults;

int index_of(uint16_t id) {
  int i = 0;
  for (const auto &app : available_apps) {
    if (app.id == id) return i;
    ++i;
  }
  return i;
}

void Init() {

  if (!app_data_storage.Load(app_settings))
    using_defaults = true;
    
  for (auto &app : available_apps)
    app.Init();
    
  global_settings.current_app_id = DEFAULT_APP_ID;
  global_settings.reserved0 = false;
  global_settings.reserved1 = false;
  using_defaults = false;

  SERIAL_PRINTLN("Load global settings: size: %u, PAGESIZE=%u, PAGES=%u, LENGTH=%u",
                sizeof(GlobalSettings),
                GlobalSettingsStorage::PAGESIZE,
                GlobalSettingsStorage::PAGES,
                GlobalSettingsStorage::LENGTH);

  if (!global_settings_storage.Load(global_settings)) {
    SERIAL_PRINTLN("Settings invalid, using defaults!");
  } else {
    SERIAL_PRINTLN("Loaded settings from page_index %d, current_app_id is %02x",
                  global_settings_storage.page_index(),global_settings.current_app_id);
  }

  SERIAL_PRINTLN("Load app data: size is %u, PAGESIZE=%u, PAGES=%u, LENGTH=%u",
                sizeof(AppData),
                AppDataStorage::PAGESIZE,
                AppDataStorage::PAGES,
                AppDataStorage::LENGTH);
  if (!app_data_storage.Load(app_settings)) {
    using_defaults = true;
    SERIAL_PRINTLN("Data not loaded, using defaults!");
  } else {
    restore_app_data();
  }
    
  int current_app_index = apps::index_of(global_settings.current_app_id);
  if (current_app_index < 0 || current_app_index >= NUM_AVAILABLE_APPS) {
    SERIAL_PRINTLN("App id %02x not found, using default!", global_settings.current_app_id);
    global_settings.current_app_id = DEFAULT_APP_INDEX;
    current_app_index = DEFAULT_APP_INDEX;
  }

  set_current_app(current_app_index);
  current_app->HandleAppEvent(APP_EVENT_RESUME);

  delay(100);
}

}; // namespace apps


void draw_save_message(uint8_t c) {
}

void Ui::SaveSettings() {
  save_app_data();
  save_global_settings(); // there aren't any ... 
}

void Ui::NukeSettings() {

  SetButtonIgnoreMask();
  apps::current_app->HandleAppEvent(APP_EVENT_SUSPEND);
  CORE::app_isr_enabled = false;
   
  // reset to defaults 
  SERIAL_PRINTLN("Erase EEPROM ...");
  EEPtr d = EEPROM_GLOBALSETTINGS_START;
  size_t len = EEPROMStorage::LENGTH - EEPROM_GLOBALSETTINGS_START;
  while (len--)
    *d++ = 0;
  SERIAL_PRINTLN("...done");
  global_settings_storage.Init();
  app_data_storage.Init();
  
  event_queue_.Flush();
  event_queue_.Poke();
  delay(1000);

  // Restore state
  apps::current_app->HandleAppEvent(APP_EVENT_RESUME);
  CORE::app_isr_enabled = true;
}

}; // namespace C2M
