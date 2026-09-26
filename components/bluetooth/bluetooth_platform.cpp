#include "bluetooth/bluetooth_platform.h"

#include "blueshift/log.h"
#include "bluetooth/bt_build_config.h"

#if (defined(BLUESHIFT_BOARD_T_LION) || defined(BLUESHIFT_ENABLE_ESP_IDF_BT) || \
     defined(BLUESHIFT_NATIVE_ESP_IDF)) &&                                      \
    (defined(ARDUINO) || defined(ESP_PLATFORM)) &&                             \
    (defined(ESP32) || defined(CONFIG_IDF_TARGET_ESP32) || defined(BLUESHIFT_NATIVE_ESP_IDF))
#define BLUESHIFT_BT_PLATFORM_ACTIVE 1
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <nvs_flash.h>
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
#if defined(BLUESHIFT_NATIVE_ESP_IDF)
    // Native ESP-IDF production builds enable CONFIG_BT_HID_HOST_ENABLED in sdkconfig.
    c.classicHidHostApi = true;
    c.bleHidDeviceApi = true;
#endif
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
        lastError_ = BluetoothError::AlreadyStarted;
        return status_ == BluetoothPlatformStatus::Ready ||
               status_ == BluetoothPlatformStatus::Partial;
    }

    caps_ = probeCapabilities();

#if !BLUESHIFT_BT_PLATFORM_ACTIVE
    status_ = BluetoothPlatformStatus::Failed;
    lastError_ = BluetoothError::UnsupportedBuild;
    detail_ = "BT platform inactive on this build (need T-Lion/native ESP-IDF)";
    BS_LOG_INFO("BOOT", "BluetoothPlatform: %s", detail_);
    return false;
#else
#if defined(BLUESHIFT_NATIVE_ESP_IDF)
    // NVS is required by Bluedroid bonding — use namespace ownership elsewhere for app config.
    esp_err_t nvsErr = nvs_flash_init();
    if (nvsErr == ESP_ERR_NVS_NO_FREE_PAGES || nvsErr == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Controlled erase of NVS partition only when corrupt — not a factory-reset path.
        nvs_flash_erase();
        nvsErr = nvs_flash_init();
    }
    if (nvsErr != ESP_OK) {
        status_ = BluetoothPlatformStatus::Failed;
        lastError_ = BluetoothError::NvsInitFailed;
        detail_ = "nvs_flash_init failed";
        BS_LOG_ERROR("BOOT", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(nvsErr));
        return false;
    }
#endif

    if (!bluedroidEnabled()) {
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        esp_err_t err = esp_bt_controller_init(&bt_cfg);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            lastError_ = BluetoothError::ControllerInitFailed;
            detail_ = "esp_bt_controller_init failed";
            BS_LOG_ERROR("BT-CLASSIC", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
        err = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            lastError_ = BluetoothError::ControllerEnableFailed;
            detail_ = "esp_bt_controller_enable(BTDM) failed";
            BS_LOG_ERROR("BT-CLASSIC", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
        err = esp_bluedroid_init();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            lastError_ = BluetoothError::BluedroidInitFailed;
            detail_ = "esp_bluedroid_init failed";
            BS_LOG_ERROR("BLE", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
        err = esp_bluedroid_enable();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            status_ = BluetoothPlatformStatus::Failed;
            lastError_ = BluetoothError::BluedroidEnableFailed;
            detail_ = "esp_bluedroid_enable failed";
            BS_LOG_ERROR("BLE", "BluetoothPlatform: %s (%d)", detail_, static_cast<int>(err));
            return false;
        }
    }

    started_ = true;
    lastError_ = BluetoothError::None;
    if (caps_.classicHidHostApi && caps_.bleHidDeviceApi) {
        status_ = BluetoothPlatformStatus::Ready;
        detail_ = "BTDM Bluedroid dual-mode APIs available";
        BS_LOG_INFO("BOOT", "BluetoothPlatform READY (IMPLEMENTED_UNVERIFIED radio)");
        return true;
    }
    status_ = BluetoothPlatformStatus::Partial;
    if (!caps_.classicHidHostApi) {
        detail_ = "BTDM up; Classic HID Host API missing in this SDK "
                  "(need CONFIG_BT_HID_HOST_ENABLED)";
    } else {
        detail_ = "BTDM up; BLE HID Device API missing";
    }
    BS_LOG_WARN("BOOT", "BluetoothPlatform PARTIAL: %s", detail_);
    return true;
#endif
}

void BluetoothPlatform::deinitialize() {
#if BLUESHIFT_BT_PLATFORM_ACTIVE
    if (!started_) {
        lastError_ = BluetoothError::NotStarted;
        return;
    }
    // Safe reverse order when supported. IMPLEMENTED_UNVERIFIED coexistence with Arduino.
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    started_ = false;
    status_ = BluetoothPlatformStatus::Uninitialized;
    lastError_ = BluetoothError::None;
    detail_ = "deinitialized";
#else
    detail_ = "deinitialize noop";
#endif
}

} // namespace blueshift
