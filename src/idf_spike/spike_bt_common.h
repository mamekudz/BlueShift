#pragma once

/**
 * BlueShift ESP-IDF Bluetooth spike helpers.
 * Compile/link feasibility only — PHYSICAL UNVERIFIED.
 */

#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_log.h"
#include "nvs_flash.h"

static inline esp_err_t spikeNvsInit(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

static inline esp_err_t spikeControllerInit(esp_bt_mode_t mode) {
    esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_bt_controller_init(&cfg);
    if (err != ESP_OK) {
        return err;
    }
    return esp_bt_controller_enable(mode);
}

static inline esp_err_t spikeBluedroidInit(void) {
    esp_err_t err = esp_bluedroid_init();
    if (err != ESP_OK) {
        return err;
    }
    return esp_bluedroid_enable();
}
