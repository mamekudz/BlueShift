/**
 * BlueShift native ESP-IDF spike — Classic HID Host only.
 *
 * Success criterion: esp_bt_hid_host_* symbols compile and link.
 * No scanning/pairing. PHYSICAL UNVERIFIED.
 */

#include "spike_bt_common.h"

#include "esp_hidh_api.h"

static const char *TAG = "SPIKE-CLASSIC";

static void classicHidHostCallback(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param) {
    (void)event;
    (void)param;
}

void app_main(void) {
    ESP_LOGI(TAG, "Classic HID Host spike start");
    ESP_ERROR_CHECK(spikeNvsInit());
    ESP_ERROR_CHECK(spikeControllerInit(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(spikeBluedroidInit());

    ESP_ERROR_CHECK(esp_bt_hid_host_register_callback(classicHidHostCallback));
    ESP_ERROR_CHECK(esp_bt_hid_host_init());
    ESP_LOGI(TAG, "Classic HID Host symbols: init=%p register=%p connect=%p disconnect=%p",
             (void *)esp_bt_hid_host_init, (void *)esp_bt_hid_host_register_callback,
             (void *)esp_bt_hid_host_connect, (void *)esp_bt_hid_host_disconnect);
    ESP_LOGI(TAG, "Classic HID Host: LINKED (init OK)");
    /* Keep running so the binary is a real app image; no radio activity. */
}
