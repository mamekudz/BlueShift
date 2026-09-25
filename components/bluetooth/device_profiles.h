#pragma once

// Device profiles — evidence-backed metadata + parsers in hid/device_parsers.*.
// Status: IMPLEMENTED_UNVERIFIED until physical capture on BlueShift hardware.

namespace blueshift {
namespace profiles {

struct DeviceProfileMeta {
    const char *id;
    const char *manufacturer;
    const char *model;
    const char *transportHint;
    const char *notes;
};

inline constexpr DeviceProfileMeta kGenericKeyboard = {
    "generic-hid-keyboard",
    "Generic",
    "HID Keyboard",
    "classic-br-edr",
    "Boot-protocol 8-byte report (mod+6KRO). Industry-standard layout."};

inline constexpr DeviceProfileMeta kSn30Pro = {
    "8bitdo-sn30-pro",
    "8BitDo",
    "SN30 Pro",
    "classic-br-edr",
    "VID 0x2DC8 DOCUMENTED. Report map from Bluepad32/community — mode-dependent. "
    "Rumble/battery UNKNOWN on BlueShift until capture. IMPLEMENTED_UNVERIFIED."};

inline constexpr DeviceProfileMeta kNimbus69070 = {
    "steelseries-nimbus-69070",
    "SteelSeries",
    "Nimbus 69070",
    "classic-br-edr",
    "Prior ESP][ evidence: NOT visible to ESP32-S3 BLE scanner; visible to Windows BT. "
    "Classic candidate. Axes/report map incomplete — minimal digital bits only."};

} // namespace profiles
} // namespace blueshift
