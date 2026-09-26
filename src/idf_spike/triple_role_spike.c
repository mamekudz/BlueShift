/**
 * BlueShift TRIPLE-ROLE spike:
 *   Classic HID Host + BLE HID Device + A2DP Source
 * ONE Bluedroid BTDM stack. No NimBLE. No BTstack.
 * Compile/link proof only — PHYSICAL UNVERIFIED. Does not alter production app.
 */

#include "spike_bt_common.h"

#include "esp_a2dp_api.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_hidh_api.h"
#include "esp_hidd.h"

static const char *TAG = "SPIKE-TRIPLE";

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

static void a2dCallback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    (void)event;
    (void)param;
}

static int32_t a2dDataCallback(uint8_t *buf, int32_t len) {
    if (len < 0 || buf == NULL) {
        return 0;
    }
    for (int32_t i = 0; i < len; ++i) {
        buf[i] = 0;
    }
    return len;
}

void app_main(void) {
    ESP_LOGI(TAG, "Triple-role spike: Classic HID Host + BLE HID + A2DP Source");
    ESP_ERROR_CHECK(spikeNvsInit());
    ESP_ERROR_CHECK(spikeControllerInit(ESP_BT_MODE_BTDM));
    ESP_ERROR_CHECK(spikeBluedroidInit());

    /* 1) Classic HID Host */
    ESP_ERROR_CHECK(esp_bt_hid_host_register_callback(classicHidHostCallback));
    ESP_ERROR_CHECK(esp_bt_hid_host_init());

    /* 2) BLE HID Device */
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(bleGapEventHandler));
    ESP_ERROR_CHECK(esp_ble_gap_set_device_name("BlueShiftTriple"));
    static esp_hid_raw_report_map_t reportMap = {
        .data = kHidReportMap,
        .len = sizeof(kHidReportMap),
    };
    static esp_hid_device_config_t hidConfig = {
        .vendor_id = 0x16C0,
        .product_id = 0x05DF,
        .version = 0x0100,
        .device_name = "BlueShiftTriple",
        .manufacturer_name = "BlueShift",
        .serial_number = "SPIKE-TRIPLE",
        .report_maps = &reportMap,
        .report_maps_len = 1,
    };
    esp_hidd_dev_t *hidDev = NULL;
    ESP_ERROR_CHECK(esp_hidd_dev_init(&hidConfig, ESP_HID_TRANSPORT_BLE, bleHidEventCallback, &hidDev));

    /* 3) A2DP Source */
    ESP_ERROR_CHECK(esp_a2d_register_callback(a2dCallback));
    ESP_ERROR_CHECK(esp_a2d_source_register_data_callback(a2dDataCallback));
    ESP_ERROR_CHECK(esp_a2d_source_init());

    /* Force-retain symbols for ELF evidence (connect unused at runtime in spike). */
    ESP_LOGI(TAG, "CLASSIC_HID_HOST LINKED init=%p connect=%p disconnect=%p",
             (void *)esp_bt_hid_host_init, (void *)esp_bt_hid_host_connect,
             (void *)esp_bt_hid_host_disconnect);
    ESP_LOGI(TAG, "BLE_HID_DEVICE LINKED dev=%p hidd_init=%p", (void *)hidDev,
             (void *)esp_hidd_dev_init);
    ESP_LOGI(TAG, "A2DP_SOURCE LINKED init=%p data_cb=%p connect=%p disconnect=%p",
             (void *)esp_a2d_source_init, (void *)esp_a2d_source_register_data_callback,
             (void *)esp_a2d_source_connect, (void *)esp_a2d_source_disconnect);

    ESP_LOGI(TAG, "Triple-role compile/link proof complete (runtime coexistence UNVERIFIED)");
}
