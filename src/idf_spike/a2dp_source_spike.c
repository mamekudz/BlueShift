/**
 * BlueShift A2DP Source-only spike — compile/link proof.
 * Synthetic PCM callback; no physical speaker required.
 * PHYSICAL UNVERIFIED.
 */

#include "spike_bt_common.h"

#include "esp_a2dp_api.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

static const char *TAG = "SPIKE-A2DP";

static void a2dCallback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    (void)event;
    (void)param;
}

/* Pull-model PCM: fill buf with silence/synthetic tone samples. */
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
    ESP_LOGI(TAG, "A2DP Source spike start");
    ESP_ERROR_CHECK(spikeNvsInit());
    /* Classic-only is enough for A2DP-only proof; BTDM also acceptable. */
    ESP_ERROR_CHECK(spikeControllerInit(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(spikeBluedroidInit());

    ESP_ERROR_CHECK(esp_bt_gap_set_device_name("BlueShiftA2dpSpike"));
    ESP_ERROR_CHECK(esp_a2d_register_callback(a2dCallback));
    ESP_ERROR_CHECK(esp_a2d_source_register_data_callback(a2dDataCallback));
    ESP_ERROR_CHECK(esp_a2d_source_init());

    ESP_LOGI(TAG, "A2DP Source LINKED init=%p data_cb_reg=%p connect=%p disconnect=%p",
             (void *)esp_a2d_source_init, (void *)esp_a2d_source_register_data_callback,
             (void *)esp_a2d_source_connect, (void *)esp_a2d_source_disconnect);
    ESP_LOGI(TAG, "A2DP Source compile/link proof complete");
}
