#pragma once

// Device profile placeholders — facts only, no fabricated report layouts.

namespace blueshift {
namespace profiles {

struct DeviceProfileMeta {
    const char *id;
    const char *manufacturer;
    const char *model;
    const char *transportHint; // classic-br-edr | ble | ...
    const char *notes;
};

inline constexpr DeviceProfileMeta kSn30Pro = {
    "8bitdo-sn30-pro",
    "8BitDo",
    "SN30 Pro",
    "classic-br-edr",
    "CANDIDATE BlueShift Classic input. Report layout TBD — physical capture required."};

inline constexpr DeviceProfileMeta kNimbus69070 = {
    "steelseries-nimbus-69070",
    "SteelSeries",
    "Nimbus 69070",
    "classic-br-edr",
    "CANDIDATE Classic input. ESP][ BLE scan INCOMPATIBLE (prior physical evidence)."};

} // namespace profiles
} // namespace blueshift
