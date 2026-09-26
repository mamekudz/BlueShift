#include "bluetooth/esp_idf_classic_hid_host.h"

#include "bluetooth/bt_build_config.h"
#include "blueshift/log.h"

#if (BLUESHIFT_HAVE_CLASSIC_HID_HOST_API || defined(BLUESHIFT_NATIVE_ESP_IDF)) && \
    (defined(ARDUINO) || defined(ESP_PLATFORM)) &&                               \
    (defined(ESP32) || defined(CONFIG_IDF_TARGET_ESP32) || defined(BLUESHIFT_NATIVE_ESP_IDF))
#define BLUESHIFT_CLASSIC_HID_RADIO 1
#include <esp_gap_bt_api.h>
#include <esp_hidh_api.h>
#else
#define BLUESHIFT_CLASSIC_HID_RADIO 0
#endif

namespace blueshift {

bool EspIdfClassicHidHost::start() {
    const bool platOk = BluetoothPlatform::instance().initializeDualMode();
    if (!hardwareApiPresent()) {
        if (!platOk) {
            BS_LOG_INFO("BT-CLASSIC",
                        "EspIdfClassicHidHost: stub FSM (no Classic HID Host API / inactive platform)");
        } else {
            BS_LOG_WARN("BT-CLASSIC",
                        "EspIdfClassicHidHost: API not linked — stub FSM only "
                        "(see docs/bluetooth/esp-idf-dual-mode-spike.md)");
        }
        return ClassicHidHostSpike::start();
    }

#if BLUESHIFT_CLASSIC_HID_RADIO
    esp_err_t err =
        esp_bt_hid_host_register_callback([](esp_hidh_cb_event_t, esp_hidh_cb_param_t *) {});
    if (err != ESP_OK) {
        BS_LOG_ERROR("BT-CLASSIC", "esp_bt_hid_host_register_callback failed %d",
                     static_cast<int>(err));
        setStateForTest(ClassicHostState::Error);
        return false;
    }
    err = esp_bt_hid_host_init();
    if (err != ESP_OK) {
        BS_LOG_ERROR("BT-CLASSIC", "esp_bt_hid_host_init failed %d", static_cast<int>(err));
        setStateForTest(ClassicHostState::Error);
        return false;
    }
    BS_LOG_INFO("BT-CLASSIC", "EspIdfClassicHidHost init OK (IMPLEMENTED_UNVERIFIED)");
    return ClassicHidHostSpike::start();
#else
    return ClassicHidHostSpike::start();
#endif
}

bool EspIdfClassicHidHost::startPairing(uint32_t timeoutMs) {
    if (!hardwareApiPresent()) {
        return ClassicHidHostSpike::startPairing(timeoutMs);
    }
#if BLUESHIFT_CLASSIC_HID_RADIO
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
#endif
    return ClassicHidHostSpike::startPairing(timeoutMs);
}

bool EspIdfClassicHidHost::reconnect() {
    if (!hardwareApiPresent()) {
        return ClassicHidHostSpike::reconnect();
    }
    return ClassicHidHostSpike::reconnect();
}

} // namespace blueshift
