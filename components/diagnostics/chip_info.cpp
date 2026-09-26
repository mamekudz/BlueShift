#include "diagnostics/chip_info.h"

#include "blueshift/version.h"

#if defined(ESP_PLATFORM)
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#if defined(BLUESHIFT_NATIVE_ESP_IDF) && !defined(ARDUINO)
#include "esp_idf_version.h"
#if CONFIG_SPIRAM
#include "esp_psram.h"
#endif
#endif
#endif

namespace blueshift {

ChipInfoReport captureChipInfo() {
    ChipInfoReport r;
    r.idfVersion = "host";
#if defined(ESP_PLATFORM)
#if defined(BLUESHIFT_NATIVE_ESP_IDF) && !defined(ARDUINO)
    r.idfVersion = esp_get_idf_version();
#else
    r.idfVersion = "arduino-esp32";
#endif
    esp_chip_info_t chip{};
    esp_chip_info(&chip);
    r.cores = chip.cores;
    r.revision = chip.revision;
    switch (chip.model) {
    case CHIP_ESP32:
        r.model = "ESP32";
        break;
    case CHIP_ESP32S2:
        r.model = "ESP32-S2";
        break;
    case CHIP_ESP32S3:
        r.model = "ESP32-S3";
        break;
    case CHIP_ESP32C3:
        r.model = "ESP32-C3";
        break;
    default:
        r.model = "unknown";
        break;
    }
    uint32_t flash = 0;
    if (esp_flash_get_size(nullptr, &flash) == ESP_OK) {
        r.flashSizeBytes = flash;
    }
#if defined(BLUESHIFT_NATIVE_ESP_IDF) && !defined(ARDUINO) && CONFIG_SPIRAM
    r.psramPresent = esp_psram_is_initialized();
    if (r.psramPresent) {
        r.psramSizeBytes = static_cast<uint32_t>(esp_psram_get_size());
    }
#elif defined(BOARD_HAS_PSRAM)
    r.psramPresent = true;
    // Size unknown on Arduino path until physical probe — leave 0 (UNKNOWN).
#endif
    r.freeHeap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    r.minFreeHeap = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
#else
    (void)BLUESHIFT_VERSION_STRING;
    r.model = "host-sim";
#endif
    return r;
}

} // namespace blueshift
