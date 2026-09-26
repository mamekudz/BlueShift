/**
 * BlueShift native ESP-IDF spike — Classic HID Host + BLE HID Device.
 *
 * Critical proof: ONE binary, ONE Bluedroid/BTDM stack, BOTH roles linked.
 * No scanning/pairing/bridging. PHYSICAL UNVERIFIED.
 *
 * Uses only ESP-IDF component APIs (not example-local esp_hid_gap).
 */

#include "spike_bt_common.h"

#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_hidh_api.h"
#include "esp_hidd.h"

static const char *TAG = "SPIKE-DUAL";

static const uint8_t kHidReportMap[] = {
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x85, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00,
    0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x03, 0x95, 0x05,
    0x75, 0x01, 0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x03,
    0x95, 0x06, 0x75, 0x08, 0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00, 0x29, 0x65, 0x81, 0x00,
    0xC0,
};

static void classicHidHostCallback(esp_hidh_cb_event_t event, esp_hidh_cb_param_t *param) {
    (void)event;
    (void)param;
}

static void bleGapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    (void)event;
    (void)param;
}

static void bleHidEventCallback(void *handlerArgs, esp_event_base_t base, int32_t id,
                                void *eventData) {
    (void)handlerArgs;
    (void)base;
    (void)id;
    (void)eventData;
}

void app_main(void) {
    ESP_LOGI(TAG, "Dual-mode HID spike start (Classic Host + BLE Device)");
    ESP_ERROR_CHECK(spikeNvsInit());
    ESP_ERROR_CHECK(spikeControllerInit(ESP_BT_MODE_BTDM));
    ESP_ERROR_CHECK(spikeBluedroidInit());

    /* Classic HID Host — Bluedroid API (must appear in final ELF). */
    ESP_ERROR_CHECK(esp_bt_hid_host_register_callback(classicHidHostCallback));
    ESP_ERROR_CHECK(esp_bt_hid_host_init());
    ESP_LOGI(TAG, "Classic HID Host: LINKED init=%p register=%p connect=%p disconnect=%p",
             (void *)esp_bt_hid_host_init, (void *)esp_bt_hid_host_register_callback,
             (void *)esp_bt_hid_host_connect, (void *)esp_bt_hid_host_disconnect);

    /* BLE HID Device — esp_hid component (must appear in final ELF). */
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(bleGapEventHandler));
    ESP_ERROR_CHECK(esp_ble_gap_set_device_name("BlueShiftSpike"));

    static esp_hid_raw_report_map_t reportMap = {
        .data = kHidReportMap,
        .len = sizeof(kHidReportMap),
    };
    static esp_hid_device_config_t hidConfig = {
        .vendor_id = 0x16C0,
        .product_id = 0x05DF,
        .version = 0x0100,
        .device_name = "BlueShiftSpike",
        .manufacturer_name = "BlueShift",
        .serial_number = "SPIKE-DUAL",
        .report_maps = &reportMap,
        .report_maps_len = 1,
    };

    esp_hidd_dev_t *hidDev = NULL;
    ESP_ERROR_CHECK(esp_hidd_dev_init(&hidConfig, ESP_HID_TRANSPORT_BLE, bleHidEventCallback, &hidDev));
    ESP_LOGI(TAG, "BLE HID Device: LINKED dev=%p", (void *)hidDev);

    ESP_LOGI(TAG, "Dual-mode compile/link proof complete");
}
