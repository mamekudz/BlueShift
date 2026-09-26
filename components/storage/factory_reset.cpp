#include "storage/factory_reset.h"

namespace blueshift {

FactoryResetResult FactoryReset::run(const FactoryResetHooks &hooks, FactoryResetScope scope) {
    FactoryResetResult r;
    r.settingsCleared = true;
    r.devicesCleared = true;
    r.classicForgetOk = true;
    r.bleForgetOk = true;

    const bool doSettings =
        scope == FactoryResetScope::SettingsOnly || scope == FactoryResetScope::FullBlueShift;
    const bool doDevices = scope == FactoryResetScope::FullBlueShift;
    const bool doClassic =
        scope == FactoryResetScope::ForgetClassic || scope == FactoryResetScope::FullBlueShift;
    const bool doBle =
        scope == FactoryResetScope::ForgetBle || scope == FactoryResetScope::FullBlueShift;

    if (doSettings) {
        if (hooks.config != nullptr) {
            r.settingsCleared = hooks.config->clear();
            const AppConfigV1 fresh = makeDefaultConfigV1();
            hooks.config->save(fresh);
        }
    }

    if (doDevices) {
        if (hooks.devices != nullptr) {
            r.devicesCleared = hooks.devices->clear();
        }
    }

    if (doClassic) {
        if (hooks.classic != nullptr) {
            r.classicForgetOk = hooks.classic->forgetDevice();
        }
    }

    if (doBle) {
        if (hooks.ble != nullptr) {
            r.bleForgetOk = hooks.ble->forgetHost();
        }
    }

    r.ok = r.settingsCleared && r.devicesCleared && r.classicForgetOk && r.bleForgetOk;
    return r;
}

} // namespace blueshift
