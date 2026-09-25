#pragma once

#include <stdint.h>

enum class BlueshiftLocale : uint8_t {
    EnUs = 0,
    DeDe = 1,
};

enum class BlueshiftMsgId : uint16_t {
    BootSkeleton = 0,
    StatusPlanned = 1,
    StatusUnverified = 2,
    ScreenStatus = 3,
    ScreenPairInput = 4,
    ScreenPairHost = 5,
    ScreenDevices = 6,
    ScreenProfiles = 7,
    ScreenDiagnostics = 8,
    ScreenSettings = 9,
    ScreenAbout = 10,
    LabelIn = 11,
    LabelOut = 12,
    LabelBat = 13,
    LabelOk = 14,
    LabelNotOk = 15,
    LabelCharging = 16,
    LabelLowBattery = 17,
    LabelCriticalBattery = 18,
    ConfirmFactoryReset = 19,
    Cancel = 20,
    MsgCount = 21
};

void i18nSetLocale(BlueshiftLocale locale);
BlueshiftLocale i18nGetLocale();
const char *i18nMsg(BlueshiftMsgId id);

// Validation helpers (host tests).
bool i18nHasFallback(BlueshiftMsgId id);
unsigned i18nMessageCount();
