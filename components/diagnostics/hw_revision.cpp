#include "diagnostics/hw_revision.h"

#include <cstring>

#include "blueshift/log.h"

namespace blueshift {

HwRevisionCompare compareDocumentedVsDetected(const ChipInfoReport &detected) {
    HwRevisionCompare c;
    c.detected = detected;
    c.socLooksEsp32 = (detected.model != nullptr && std::strcmp(detected.model, "ESP32") == 0);
    // Flash: allow ±0 until physical probe; treat exact 4 MiB as match to ASSUMED.
    c.flashMatchesAssumed =
        (detected.flashSizeBytes == c.documented.flashBytesAssumed) ||
        (detected.flashSizeBytes == 0); // unknown until chip probe
    c.psramPresentMatches = detected.psramPresent == c.documented.psramExpected ||
                            detected.psramSizeBytes == 0;
    return c;
}

void emitHwRevisionReport(const ChipInfoReport &detected) {
    const HwRevisionCompare c = compareDocumentedVsDetected(detected);
    BS_LOG_INFO("DIAG", "hw DOCUMENTED module=%s flash=%u psram=%u", c.documented.module,
                static_cast<unsigned>(c.documented.flashBytesAssumed),
                static_cast<unsigned>(c.documented.psramBytesAssumed));
    BS_LOG_INFO("DIAG", "hw DETECTED model=%s rev=%u cores=%u flash=%u psram=%u heap=%u",
                detected.model != nullptr ? detected.model : "?",
                static_cast<unsigned>(detected.revision),
                static_cast<unsigned>(detected.cores),
                static_cast<unsigned>(detected.flashSizeBytes),
                static_cast<unsigned>(detected.psramSizeBytes),
                static_cast<unsigned>(detected.freeHeap));
    BS_LOG_INFO("DIAG", "hw compare soc_esp32=%d flash_assumed_match=%d psram_present_ok=%d "
                        "(PHYSICAL VERIFIED: no)",
                c.socLooksEsp32 ? 1 : 0, c.flashMatchesAssumed ? 1 : 0,
                c.psramPresentMatches ? 1 : 0);
}

} // namespace blueshift
