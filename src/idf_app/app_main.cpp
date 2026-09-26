/**
 * BlueShift™ native ESP-IDF production entry.
 * Orchestrates project-owned components — keep this file small.
 * PHYSICAL UNVERIFIED until T-Lion bring-up.
 */

#include "app/application.h"

#include "blueshift/log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

blueshift::Application gApp;

void appLoopTask(void *) {
    while (true) {
        const uint32_t nowMs = static_cast<uint32_t>(xTaskGetTickCount() * portTICK_PERIOD_MS);
        gApp.loopOnce(nowMs);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

} // namespace

extern "C" void app_main(void) {
    BS_LOG_INFO("BOOT", "app_main enter");
    if (!gApp.begin()) {
        BS_LOG_ERROR("BOOT", "Application::begin failed");
    }
    // Single app loop task — avoid one FreeRTOS task per component.
    xTaskCreate(appLoopTask, "bs_app", 8192, nullptr, 5, nullptr);
}
