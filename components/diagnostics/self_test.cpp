#include "diagnostics/self_test.h"

namespace blueshift {

const char *selfTestIdName(SelfTestId id) {
    switch (id) {
    case SelfTestId::Oled:
        return "OLED";
    case SelfTestId::Nav:
        return "NAV";
    case SelfTestId::Battery:
        return "BAT";
    case SelfTestId::Classic:
        return "CLASSIC";
    case SelfTestId::Ble:
        return "BLE";
    default:
        return "UNKNOWN";
    }
}

const char *selfTestStatusName(SelfTestStatus status) {
    switch (status) {
    case SelfTestStatus::NotTested:
        return "NOT_TESTED";
    case SelfTestStatus::Ok:
        return "OK";
    case SelfTestStatus::Warn:
        return "WARN";
    case SelfTestStatus::Fail:
        return "FAIL";
    default:
        return "UNKNOWN";
    }
}

void selfTestSet(SelfTestReport &report, SelfTestId id, SelfTestStatus status, const char *detail) {
    const unsigned idx = static_cast<unsigned>(id);
    if (idx >= static_cast<unsigned>(SelfTestId::Count)) {
        return;
    }
    report.entries[idx].id = id;
    report.entries[idx].status = status;
    report.entries[idx].detail = detail != nullptr ? detail : "";
}

} // namespace blueshift
