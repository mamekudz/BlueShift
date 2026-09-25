#include <Arduino.h>

#include "blueshift/version.h"

// Milestone 1: boot skeleton only.
// No Classic BT, BLE HID, bridge, OLED, or battery bring-up yet.

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.printf("[BOOT] BlueShift %s\n", BLUESHIFT_VERSION_STRING);
#if BLUESHIFT_BUILD_TYPE_DEBUG
    Serial.println("[BOOT] build=debug (provisional skeleton target)");
#else
    Serial.println("[BOOT] build=release (provisional skeleton target)");
#endif
    Serial.println("[BOOT] T-Lion hardware bring-up: PLANNED / UNVERIFIED");
    Serial.println("[BOOT] Classic BT / BLE HID / bridge: not started");
}

void loop() {
    delay(1000);
}
