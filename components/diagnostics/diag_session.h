#pragma once

#include <cstddef>
#include <cstdint>

namespace blueshift {

enum class DiagResult : uint8_t {
    Unknown = 0,
    Ok,
    NotOk,
    Skipped
};

enum class DiagId : uint16_t {
    Oled = 1,
    Buttons = 2,
    Battery = 3,
    ClassicPairing = 4,
    ClassicHid = 5,
    BleOutput = 6,
    Bridge = 7,
    Reconnect = 8
};

struct DiagRecord {
    DiagId id = DiagId::Oled;
    DiagResult result = DiagResult::Unknown;
    int32_t metric = 0;      // language-neutral measurement
    uint32_t timestampMs = 0;
};

// Fixed RAM buffer — no microSD assumed on T-Lion.
template <std::size_t Capacity = 32>
class DiagSession {
public:
    bool add(const DiagRecord &rec) {
        if (count_ >= Capacity) {
            ++dropped_;
            return false;
        }
        records_[count_++] = rec;
        return true;
    }

    std::size_t count() const {
        return count_;
    }

    std::uint32_t dropped() const {
        return dropped_;
    }

    const DiagRecord *data() const {
        return records_;
    }

    void clear() {
        count_ = 0;
        dropped_ = 0;
    }

private:
    DiagRecord records_[Capacity] = {};
    std::size_t count_ = 0;
    std::uint32_t dropped_ = 0;
};

} // namespace blueshift
