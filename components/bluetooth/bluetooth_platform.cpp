#include "bluetooth/bluetooth_platform.h"

#include "blueshift/log.h"
#include "bluetooth/bt_build_config.h"

#if (defined(BLUESHIFT_BOARD_T_LION) || defined(BLUESHIFT_ENABLE_ESP_IDF_BT)) && defined(ARDUINO) && \
    defined(ESP32)
#define BLUESHIFT_BT_PLATFORM_ACTIVE 1
#include <esp_bt.h>
#include <esp_bt_main.h>
#else
#define BLUESHIFT_BT_PLATFORM_ACTIVE 0
#endif

namespace blueshift {

BluetoothPlatform &BluetoothPlatform::instance() {
    static BluetoothPlatform plat;
    return plat;
}

BluetoothPlatformCapabilities BluetoothPlatform::probeCapabilities() const {
    BluetoothPlatformCapabilities c;
#if BLUESHIFT_BT_PLATFORM_ACTIVE
    c.controllerBtdm = true;
    c.classicEnabled = true;
    c.bleEnabled = true;
    c.classicHidHostApi = (BLUESHIFT_HAVE_CLASSIC_HID_HOST_API != 0);
    c.bleHidDeviceApi = (BLUESHIFT_HAVE_BLE_HID_DEVICE_API != 0);
#endif
    return c;
}

bool BluetoothPlatform::bluedroidEnabled() const {
#if BLUESHIFT_BT_PLATFORM_ACTIVE
    return esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_ENABLED;
#else
    return false;
#endif
}

bool BluetoothPlatform::initializeDualMode() {
    if (started_) {
        return status_ == BluetoothPlatformStatus::Ready ||
               status_ == BluetoothPlatformStatus::Partial;
    }

    caps_ = probeCapabilities();

#if !BLUESHIFT_BT_PLATFORM_ACTIVE
    status_ = BluetoothPlatformStatus::Failed;
    detail_ = "BT platform inactive on this build (need T-Lion or BLUESHIFT_ENABLE_ESP_IDF_BT)";
    BS_LOG_INFO("BOOT", "BluetoothPlatform: %s", detail_);
    return false;
#else
    // Coherent Bluedroid BTDM only — never start NimBLE here.
    // Do not call esp_bt_controller_mem_release() — that would drop Classic or BLE.
    if (!bluedroidEnabled()) {
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        esp_err_t err = esp_bt_controller_init(&bt_cfg);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            detail_ = "esp_bt_controller_init failed";
            BS_LOG_ERROR("BT-CLASSIC", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
        err = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            detail_ = "esp_bt_controller_enable(BTDM) failed";
            BS_LOG_ERROR("BT-CLASSIC", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
        err = esp_bluedroid_init();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            detail_ = "esp_bluedroid_init failed";
            BS_LOG_ERROR("BLE", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
        err = esp_bluedroid_enable();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            detail_ = "esp_bluedroid_enable failed";
            BS_LOG_ERROR("BLE", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
    }

    started_ = true;
    if (caps_.classicHidHostApi && caps_.bleHidDeviceApi) {
        status_ = BluetoothPlatformStatus::Ready;
        detail_ = "BTDM Bluedroid dual-mode APIs available";
        BS_LOG_INFO("BOOT", "BluetoothPlatform READY (IMPLEMENTED_UNVERIFIED radio)");
        return true;
    }
    status_ = BluetoothPlatformStatus::Partial;
    if (!caps_.classicHidHostApi) {
        detail_ = "BTDM up; Classic HID Host API missing in this Arduino SDK "
                  "(need ESP-IDF sdkconfig CONFIG_BT_HID_HOST_ENABLED)";
    } else {
        detail_ = "BTDM up; BLE HID Device API missing";
    }
    BS_LOG_WARN("BOOT", "BluetoothPlatform PARTIAL: %s", detail_);
    return true;
#endif
}

void BluetoothPlatform::deinitialize() {
    detail_ = "deinitialize deferred (Arduino coexistence)";
}

} // namespace blueshift
