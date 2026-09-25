#include <Arduino.h>

#include "blueshift/log.h"
#include "blueshift/version.h"
#include "bridge/bridge_core.h"
#include "hardware/t_lion/board_pins.h"
#include "i18n.h"
#include "power/power_policy.h"

// Milestone 2: preparatory firmware skeleton.
// No physical OLED/BT bring-up claimed. Pins are DOCUMENTED only.

using blueshift::BridgeCore;
using blueshift::BridgeEvent;
using blueshift::PowerPolicy;

static BridgeCore g_bridge;
static PowerPolicy g_power;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();

    i18nSetLocale(BlueshiftLocale::EnUs);
    BS_LOG_INFO("BOOT", "BlueShift %s", BLUESHIFT_VERSION_STRING);
#if BLUESHIFT_BUILD_TYPE_DEBUG
    BS_LOG_INFO("BOOT", "build=debug");
#else
    BS_LOG_INFO("BOOT", "build=release");
#endif

#if defined(BLUESHIFT_BOARD_T_LION)
    BS_LOG_INFO("BOOT", "board env=t-lion (IMPLEMENTED_UNVERIFIED)");
    BS_LOG_INFO("OLED", "SSD1306 addr=0x%02X SDA=%d SCL=%d (DOCUMENTED)",
                blueshift::t_lion::kOledI2cAddress, blueshift::t_lion::kOledSda,
                blueshift::t_lion::kOledScl);
    BS_LOG_INFO("INPUT", "5-way up=%d down=%d ok=%d left=%d right=%d (DOCUMENTED)",
                blueshift::t_lion::kBtnUp, blueshift::t_lion::kBtnDown,
                blueshift::t_lion::kBtnConfirm, blueshift::t_lion::kBtnLeft,
                blueshift::t_lion::kBtnRight);
    BS_LOG_INFO("BATTERY", "ADC GPIO%d divider=%.1f charger=%s (DOCUMENTED/ASSUMED)",
                blueshift::t_lion::kBatteryAdc, blueshift::t_lion::kBatteryDividerRatio,
                blueshift::t_lion::kChargerIc);
#else
    BS_LOG_INFO("BOOT", "board env=skeleton/provisional");
#endif

    BS_LOG_INFO("BOOT", "%s", i18nMsg(BlueshiftMsgId::StatusUnverified));
    BS_LOG_INFO("BT-CLASSIC", "stack not linked (see docs/bluetooth/)");
    BS_LOG_INFO("BLE", "stack not linked (see docs/bluetooth/)");

    g_bridge.apply(BridgeEvent::BootDone);
    g_power.onBootDone();
    BS_LOG_INFO("BRIDGE", "state=%s", blueshift::bridgeStateName(g_bridge.state()));
    BS_LOG_INFO("POWER", "state=%s", blueshift::powerStateName(g_power.state()));
}

void loop() {
    delay(1000);
}
