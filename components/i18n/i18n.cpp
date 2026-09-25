#include "i18n.h"

#include "locale_de_de.h"
#include "locale_en_us.h"

namespace {

BlueshiftLocale g_locale = BlueshiftLocale::EnUs;

const char *const *tableFor(BlueshiftLocale locale) {
    switch (locale) {
    case BlueshiftLocale::DeDe:
        return kLocaleDeDe;
    case BlueshiftLocale::EnUs:
    default:
        return kLocaleEnUs;
    }
}

} // namespace

void i18nSetLocale(BlueshiftLocale locale) {
    g_locale = locale;
}

BlueshiftLocale i18nGetLocale() {
    return g_locale;
}

unsigned i18nMessageCount() {
    return static_cast<unsigned>(BlueshiftMsgId::MsgCount);
}

bool i18nHasFallback(BlueshiftMsgId id) {
    const auto index = static_cast<uint16_t>(id);
    if (index >= static_cast<uint16_t>(BlueshiftMsgId::MsgCount)) {
        return false;
    }
    return kLocaleEnUs[index] != nullptr && kLocaleEnUs[index][0] != '\0';
}

const char *i18nMsg(BlueshiftMsgId id) {
    const auto index = static_cast<uint16_t>(id);
    if (index >= static_cast<uint16_t>(BlueshiftMsgId::MsgCount)) {
        return "";
    }
    const char *text = tableFor(g_locale)[index];
    if (text == nullptr || text[0] == '\0') {
        text = kLocaleEnUs[index];
    }
    return text != nullptr ? text : "";
}
