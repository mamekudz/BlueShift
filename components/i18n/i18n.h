#pragma once

// Minimal i18x-facing entry point for future OLED / UI strings.
// Not a full localization engine — see components/i18n/README.md.

#include <stdint.h>

enum class BlueshiftLocale : uint8_t {
    EnUs = 0,
    DeDe = 1,
};

enum class BlueshiftMsgId : uint16_t {
    BootSkeleton = 0,
    StatusPlanned = 1,
    StatusUnverified = 2,
};

// Active locale (default: English technical fallback).
void i18nSetLocale(BlueshiftLocale locale);
BlueshiftLocale i18nGetLocale();

// Returns a flash-resident NUL-terminated string for the active locale.
// Unknown IDs fall back to en-US, then to an empty string.
const char *i18nMsg(BlueshiftMsgId id);
