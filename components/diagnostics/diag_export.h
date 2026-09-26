#pragma once

#include <cstdio>

#include "bridge/bridge_core.h"
#include "battery/battery_monitor.h"

namespace blueshift {

// Line-oriented diagnostic export over serial. Machine fields are NOT localized.
class DiagExporter {
public:
    // Emits: [DIAG-JSON] {...}
    static void emitBridge(BridgeState state, bool inputOk, bool outputOk) {
        std::printf("[DIAG-JSON] {\"type\":\"bridge\",\"state\":\"%s\",\"input\":%s,\"output\":%s}\n",
                    bridgeStateName(state), inputOk ? "true" : "false", outputOk ? "true" : "false");
    }

    static void emitBattery(const BatteryStatus &bat) {
        std::printf(
            "[DIAG-JSON] {\"type\":\"battery\",\"valid\":%s,\"mv\":%.0f,\"pct\":%u,\"health\":%u}\n",
            bat.valid ? "true" : "false", static_cast<double>(bat.voltageMv),
            static_cast<unsigned>(bat.percent), static_cast<unsigned>(bat.health));
    }

    static void emitSelfTest(const char *subsystem, bool ok, const char *note) {
        std::printf("[DIAG-JSON] {\"type\":\"selftest\",\"subsystem\":\"%s\",\"ok\":%s,\"note\":\"%s\"}\n",
                    subsystem != nullptr ? subsystem : "", ok ? "true" : "false",
                    note != nullptr ? note : "");
    }

    static void emitHid(const char *dir, const char *kind, unsigned seq) {
        std::printf("[DIAG-JSON] {\"type\":\"hid\",\"dir\":\"%s\",\"kind\":\"%s\",\"seq\":%u}\n",
                    dir != nullptr ? dir : "", kind != nullptr ? kind : "", seq);
    }

    // Machine field names are NOT localized (audio diagnostics).
    static void emitAudio(uint16_t protocolVersion, uint32_t packets, uint32_t edges,
                          uint32_t sequenceErrors, uint32_t resyncs, uint32_t underruns,
                          uint32_t overruns, uint32_t pcmSamples, uint32_t sampleRate,
                          bool audioConnected, bool streaming) {
        std::printf(
            "[DIAG-JSON] {\"type\":\"audio\",\"protocolVersion\":%u,\"packets\":%u,\"edges\":%u,"
            "\"sequenceErrors\":%u,\"resyncs\":%u,\"underruns\":%u,\"overruns\":%u,"
            "\"pcmSamples\":%u,\"sampleRate\":%u,\"audioConnected\":%s,\"streaming\":%s}\n",
            static_cast<unsigned>(protocolVersion), static_cast<unsigned>(packets),
            static_cast<unsigned>(edges), static_cast<unsigned>(sequenceErrors),
            static_cast<unsigned>(resyncs), static_cast<unsigned>(underruns),
            static_cast<unsigned>(overruns), static_cast<unsigned>(pcmSamples),
            static_cast<unsigned>(sampleRate), audioConnected ? "true" : "false",
            streaming ? "true" : "false");
    }
};

enum class DiagItem : uint8_t {
    Oled = 0,
    Buttons,
    Battery,
    ClassicScan,
    ClassicHid,
    BleOutput,
    Bridge,
    Reconnect,
    Count
};

struct DiagResult {
    DiagItem item = DiagItem::Oled;
    bool measured = false;
    bool ok = false;
    const char *note = "";
};

} // namespace blueshift
