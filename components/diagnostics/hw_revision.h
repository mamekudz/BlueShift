#pragma once

#include "diagnostics/chip_info.h"

namespace blueshift {

// DOCUMENTED expectations for T-Lion / ESP32-WROVER (not PHYSICALLY VERIFIED).
struct DocumentedHwExpectations {
    const char *module = "ESP32-WROVER";
    const char *soc = "ESP32";
    uint32_t flashBytesAssumed = 4u * 1024u * 1024u; // ASSUMED
    uint32_t psramBytesAssumed = 8u * 1024u * 1024u;  // ASSUMED
    bool psramExpected = true;                        // ASSUMED marketing
};

struct HwRevisionCompare {
    DocumentedHwExpectations documented{};
    ChipInfoReport detected{};
    bool flashMatchesAssumed = false;
    bool psramPresentMatches = false;
    bool socLooksEsp32 = false;
};

HwRevisionCompare compareDocumentedVsDetected(const ChipInfoReport &detected);

void emitHwRevisionReport(const ChipInfoReport &detected);

} // namespace blueshift
