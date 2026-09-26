#pragma once

#include <cstddef>
#include <cstdint>

namespace blueshift {

struct ChipInfoReport {
    const char *model = "";
    uint32_t revision = 0;
    uint32_t cores = 0;
    uint32_t flashSizeBytes = 0;
    uint32_t psramSizeBytes = 0;
    size_t freeHeap = 0;
    size_t minFreeHeap = 0;
    const char *idfVersion = "";
    bool psramPresent = false;
};

// Runtime detection for bring-up comparison vs DOCUMENTED values.
// PHYSICAL VERIFIED only after T-Lion execution.
ChipInfoReport captureChipInfo();

} // namespace blueshift
