#pragma once

#include <cstdint>

#include "blueshift/evidence.h"

// DOCUMENTED pin map from LilyGO TTGO-T-ControllerV2.2 / adc.ino + t18_v2.3 schematic.
// Status on our hardware: IMPLEMENTED_UNVERIFIED (not physically confirmed).

namespace blueshift {
namespace t_lion {

inline constexpr Evidence kPinEvidence = Evidence::Documented;

inline constexpr int kOledSda = 21;
inline constexpr int kOledScl = 22;
inline constexpr uint8_t kOledI2cAddress = 0x3C;
inline constexpr int kOledWidth = 128;
inline constexpr int kOledHeight = 64;

inline constexpr int kBtnUp = 32;
inline constexpr int kBtnDown = 33;
inline constexpr int kBtnConfirm = 34; // center / OK
inline constexpr int kBtnLeft = 36;
inline constexpr int kBtnRight = 39;

inline constexpr int kBatteryAdc = 35;
// Official example: ((adc/4095)*2.0*3.3*vref/1000) => 1:1 divider assumption.
inline constexpr float kBatteryDividerRatio = 2.0f;

inline constexpr int kStatusLed = 5; // ASSUMED soft / schematic IO5

inline constexpr const char *kUsbUartSchematic = "CP2104";
inline constexpr const char *kUsbUartMarketing = "CH9102"; // conflict — UNKNOWN until measured
inline constexpr const char *kChargerIc = "TP5400";
inline constexpr const char *kMcuModule = "ESP32-WROVER";

} // namespace t_lion
} // namespace blueshift
