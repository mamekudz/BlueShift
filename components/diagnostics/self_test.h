#pragma once

#include <cstdint>

namespace blueshift {

enum class SelfTestStatus : uint8_t {
    NotTested = 0,
    Ok,
    Warn,
    Fail
};

enum class SelfTestId : uint8_t {
    Oled = 0,
    Nav,
    Battery,
    Classic,
    Ble,
    Count
};

struct SelfTestEntry {
    SelfTestId id = SelfTestId::Oled;
    SelfTestStatus status = SelfTestStatus::NotTested;
    const char *detail = "";
};

struct SelfTestReport {
    SelfTestEntry entries[static_cast<unsigned>(SelfTestId::Count)]{};
};

const char *selfTestIdName(SelfTestId id);
const char *selfTestStatusName(SelfTestStatus status);

void selfTestSet(SelfTestReport &report, SelfTestId id, SelfTestStatus status, const char *detail);

} // namespace blueshift
