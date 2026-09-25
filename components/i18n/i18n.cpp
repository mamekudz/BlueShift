#include "i18n.h"

#include "locale_de_de.h"
#include "locale_en_us.h"

namespace {

BlueshiftLocale g_locale = BlueshiftLocale::EnUs;

constexpr uint16_t kMsgCount = 3;

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

const char *i18nMsg(BlueshiftMsgId id) {
    const auto index = static_cast<uint16_t>(id);
    if (index >= kMsgCount) {
        return "";
    }
    const char *text = tableFor(g_locale)[index];
    if (text == nullptr || text[0] == '\0') {
        text = kLocaleEnUs[index];
    }
    return text != nullptr ? text : "";
}
