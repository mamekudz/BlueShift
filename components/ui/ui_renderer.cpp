#include "ui/ui_renderer.h"

#include <cstdio>

#include "blueshift/version.h"
#include "bridge/bridge_core.h"
#include "i18n.h"

namespace blueshift {

const char *UiRenderer::linkGlyph(LinkIndicator link) {
    switch (link) {
    case LinkIndicator::Disconnected:
        return "--";
    case LinkIndicator::Scanning:
        return "..";
    case LinkIndicator::Pairing:
        return "<>";
    case LinkIndicator::Connected:
        return "OK";
    case LinkIndicator::Error:
        return "!!";
    }
    return "??";
}

void UiRenderer::renderMenuLine(Display &d, int y, bool selected, const char *text) const {
    char line[22];
    std::snprintf(line, sizeof(line), "%c%-18s", selected ? '>' : ' ', text != nullptr ? text : "");
    d.drawText(0, y, line);
}

void UiRenderer::renderStatus(Display &d, const UiRenderModel &m) const {
    d.drawText(0, 0, "BlueShift");
    char line[22];
    std::snprintf(line, sizeof(line), "%s %s %s", i18nMsg(BlueshiftMsgId::LabelIn),
                  linkGlyph(m.inputLink),
                  m.status.inputName != nullptr && m.status.inputName[0] ? m.status.inputName : "-");
    d.drawText(0, 14, line);
    std::snprintf(line, sizeof(line), "%s %s %s", i18nMsg(BlueshiftMsgId::LabelOut),
                  linkGlyph(m.outputLink),
                  m.status.outputName != nullptr && m.status.outputName[0] ? m.status.outputName
                                                                           : "-");
    d.drawText(0, 26, line);
    if (m.status.battery.valid && m.status.battery.percent != 0xFF) {
        std::snprintf(line, sizeof(line), "%s %u%%", i18nMsg(BlueshiftMsgId::LabelBat),
                      static_cast<unsigned>(m.status.battery.percent));
    } else {
        std::snprintf(line, sizeof(line), "%s --", i18nMsg(BlueshiftMsgId::LabelBat));
    }
    d.drawText(0, 40, line);
    d.drawText(0, 54, bridgeStateName(m.status.bridge));
}

void UiRenderer::renderPair(Display &d, BlueshiftMsgId title, LinkIndicator link) const {
    d.drawText(0, 0, i18nMsg(title));
    char line[22];
    std::snprintf(line, sizeof(line), "%s", linkGlyph(link));
    d.drawText(0, 20, line);
    d.drawText(0, 40, i18nMsg(BlueshiftMsgId::HintNavBack));
}

void UiRenderer::renderListScreen(Display &d, BlueshiftMsgId title, int selection,
                                  const BlueshiftMsgId *items, int count) const {
    d.drawText(0, 0, i18nMsg(title));
    const int visible = 4;
    int start = selection - (visible - 1);
    if (start < 0) {
        start = 0;
    }
    for (int i = 0; i < visible && start + i < count; ++i) {
        const int idx = start + i;
        renderMenuLine(d, 14 + i * 12, idx == selection, i18nMsg(items[idx]));
    }
}

void UiRenderer::renderAbout(Display &d) const {
    d.drawText(0, 0, i18nMsg(BlueshiftMsgId::ScreenAbout));
    d.drawText(0, 16, BLUESHIFT_VERSION_STRING);
    d.drawText(0, 32, i18nMsg(BlueshiftMsgId::StatusUnverified));
    d.drawText(0, 48, "T-Lion");
}

void UiRenderer::renderConfirmReset(Display &d) const {
    d.drawText(0, 8, i18nMsg(BlueshiftMsgId::ConfirmFactoryReset));
    d.drawText(0, 28, i18nMsg(BlueshiftMsgId::HintConfirm));
    d.drawText(0, 44, i18nMsg(BlueshiftMsgId::Cancel));
}

void UiRenderer::renderDiagnostics(Display &d, const UiRenderModel &m) const {
    static const BlueshiftMsgId kDiag[] = {
        BlueshiftMsgId::DiagOled,     BlueshiftMsgId::DiagButtons, BlueshiftMsgId::DiagBattery,
        BlueshiftMsgId::DiagClassic,  BlueshiftMsgId::DiagBle,     BlueshiftMsgId::DiagBridge,
        BlueshiftMsgId::DiagReconnect};
    renderListScreen(d, BlueshiftMsgId::ScreenDiagnostics, m.selection, kDiag,
                     static_cast<int>(sizeof(kDiag) / sizeof(kDiag[0])));
    if (m.diagLabel != nullptr && m.diagLabel[0] != '\0') {
        char line[22];
        std::snprintf(line, sizeof(line), "%s %s", m.diagLabel,
                      m.diagOk ? i18nMsg(BlueshiftMsgId::LabelOk)
                               : i18nMsg(BlueshiftMsgId::LabelNotOk));
        d.drawText(0, 54, line);
    }
}

void UiRenderer::render(Display &display, const UiRenderModel &model) const {
    display.clear();
    switch (model.screen) {
    case UiScreen::Status:
        renderStatus(display, model);
        break;
    case UiScreen::PairInput:
        renderPair(display, BlueshiftMsgId::ScreenPairInput, model.inputLink);
        break;
    case UiScreen::PairHost:
        renderPair(display, BlueshiftMsgId::ScreenPairHost, model.outputLink);
        break;
    case UiScreen::Devices: {
        static const BlueshiftMsgId items[] = {BlueshiftMsgId::LabelIn, BlueshiftMsgId::LabelOut};
        renderListScreen(display, BlueshiftMsgId::ScreenDevices, model.selection, items, 2);
        break;
    }
    case UiScreen::Profiles: {
        static const BlueshiftMsgId items[] = {BlueshiftMsgId::ProfileKeyboard,
                                              BlueshiftMsgId::ProfileGamepad};
        renderListScreen(display, BlueshiftMsgId::ScreenProfiles, model.selection, items, 2);
        break;
    }
    case UiScreen::Diagnostics:
        renderDiagnostics(display, model);
        break;
    case UiScreen::Settings: {
        static const BlueshiftMsgId items[] = {BlueshiftMsgId::ConfirmFactoryReset,
                                              BlueshiftMsgId::ScreenAbout,
                                              BlueshiftMsgId::LabelBat};
        renderListScreen(display, BlueshiftMsgId::ScreenSettings, model.selection, items, 3);
        break;
    }
    case UiScreen::About:
        renderAbout(display);
        break;
    case UiScreen::ConfirmFactoryReset:
        renderConfirmReset(display);
        break;
    }
    display.present();
}

} // namespace blueshift
