#pragma once

#include "display/display.h"
#include "i18n.h"
#include "ui/ui_controller.h"

namespace blueshift {

enum class LinkIndicator : uint8_t {
    Disconnected = 0,
    Scanning,
    Pairing,
    Connected,
    Error
};

struct UiRenderModel {
    UiStatusModel status{};
    UiScreen screen = UiScreen::Status;
    int selection = 0;
    LinkIndicator inputLink = LinkIndicator::Disconnected;
    LinkIndicator outputLink = LinkIndicator::Disconnected;
    LinkIndicator audioLink = LinkIndicator::Disconnected;
    const char *diagLabel = "";
    bool diagOk = false;
    bool selfTestPending = true;
    bool audioUiEnabled = false; // experimental menu; default off
};

// Renders compact 128x64 layouts. All user strings via i18x.
class UiRenderer {
public:
    void render(Display &display, const UiRenderModel &model) const;

private:
    void renderStatus(Display &d, const UiRenderModel &m) const;
    void renderMenuLine(Display &d, int y, bool selected, const char *text) const;
    void renderPair(Display &d, BlueshiftMsgId title, LinkIndicator link) const;
    void renderListScreen(Display &d, BlueshiftMsgId title, int selection,
                          const BlueshiftMsgId *items, int count) const;
    void renderAbout(Display &d) const;
    void renderConfirmReset(Display &d) const;
    void renderDiagnostics(Display &d, const UiRenderModel &m) const;
    static const char *linkGlyph(LinkIndicator link);
};

} // namespace blueshift
