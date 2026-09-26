#include "battery/adc_battery.h"

#if defined(ARDUINO)
#include <Arduino.h>
#elif defined(ESP_PLATFORM)
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#endif

namespace blueshift {

#if defined(ESP_PLATFORM) && !defined(ARDUINO)
namespace {
adc_oneshot_unit_handle_t gAdcHandle = nullptr;
adc_cali_handle_t gCaliHandle = nullptr;
bool gCaliOk = false;
adc_channel_t gAdcChannel = ADC_CHANNEL_7; // GPIO35 = ADC1_CH7 on ESP32
adc_unit_t gAdcUnit = ADC_UNIT_1;
} // namespace
#endif

AdcBatteryBackend::AdcBatteryBackend(const t_lion::BoardConfig &cfg) : cfg_(cfg) {}

bool AdcBatteryBackend::begin() {
#if defined(ARDUINO)
    analogReadResolution(12);
    ready_ = true;
    return true;
#elif defined(ESP_PLATFORM)
    if (gAdcHandle == nullptr) {
        adc_oneshot_unit_init_cfg_t unitCfg = {};
        unitCfg.unit_id = ADC_UNIT_1;
        if (adc_oneshot_new_unit(&unitCfg, &gAdcHandle) != ESP_OK) {
            return false;
        }
    }
    if (adc_oneshot_io_to_channel(cfg_.batteryAdc, &gAdcUnit, &gAdcChannel) != ESP_OK) {
        return false;
    }
    adc_oneshot_chan_cfg_t chanCfg = {};
    chanCfg.bitwidth = ADC_BITWIDTH_12;
    chanCfg.atten = ADC_ATTEN_DB_12;
    if (adc_oneshot_config_channel(gAdcHandle, gAdcChannel, &chanCfg) != ESP_OK) {
        return false;
    }

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t caliCfg = {};
    caliCfg.unit_id = gAdcUnit;
    caliCfg.chan = gAdcChannel;
    caliCfg.atten = ADC_ATTEN_DB_12;
    caliCfg.bitwidth = ADC_BITWIDTH_12;
    gCaliOk = adc_cali_create_scheme_curve_fitting(&caliCfg, &gCaliHandle) == ESP_OK;
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t caliCfg = {};
    caliCfg.unit_id = gAdcUnit;
    caliCfg.atten = ADC_ATTEN_DB_12;
    caliCfg.bitwidth = ADC_BITWIDTH_12;
    gCaliOk = adc_cali_create_scheme_line_fitting(&caliCfg, &gCaliHandle) == ESP_OK;
#else
    gCaliOk = false;
#endif
    ready_ = true;
    return true;
#else
    ready_ = false;
    return false;
#endif
}

AdcReading AdcBatteryBackend::readRaw() {
    AdcReading r;
#if defined(ARDUINO)
    if (!ready_) {
        return r;
    }
    r.raw = analogRead(cfg_.batteryAdc);
    r.millivolts =
        (static_cast<float>(r.raw) / 4095.0f) * cfg_.batteryDivider * 3300.0f * (vrefMv_ / 1000.0f) *
        calGain_;
    r.valid = r.raw >= 0 && r.raw <= 4095;
#elif defined(ESP_PLATFORM)
    if (!ready_ || gAdcHandle == nullptr) {
        return r;
    }
    int raw = 0;
    if (adc_oneshot_read(gAdcHandle, gAdcChannel, &raw) != ESP_OK) {
        return r;
    }
    r.raw = raw;
    int mv = 0;
    if (gCaliOk && gCaliHandle != nullptr &&
        adc_cali_raw_to_voltage(gCaliHandle, raw, &mv) == ESP_OK) {
        r.millivolts = static_cast<float>(mv) * cfg_.batteryDivider * calGain_;
    } else {
        // Fallback without eFuse calibration — reduced accuracy (WARN at bring-up).
        r.millivolts =
            (static_cast<float>(raw) / 4095.0f) * cfg_.batteryDivider * 3300.0f *
            (vrefMv_ / 1000.0f) * calGain_;
    }
    r.valid = true;
#else
    r.valid = false;
#endif
    return r;
}

BatterySample AdcBatteryBackend::sample() {
    BatterySample s;
    const AdcReading r = readRaw();
    s.valid = r.valid;
    s.voltageMv = r.millivolts;
    // Charging / USB detection: UNKNOWN without verified CHRG GPIO (TP5400).
    s.charging = false;
    s.externalPower = false;
    return s;
}

} // namespace blueshift
