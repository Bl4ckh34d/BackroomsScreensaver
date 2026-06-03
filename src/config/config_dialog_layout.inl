// Config dialog scrolling, tab visibility, and child layout helpers.

int ConfigVisibleHeight() {
    return kConfigContentBottom - kConfigContentTop;
}

int ConfigMaxScroll(const ConfigState* state, int tab) {
    if (!state || tab < 0 || tab >= state->tabCount) return 0;
    return std::max(0, state->contentHeight[static_cast<size_t>(tab)] - ConfigVisibleHeight());
}

void MoveConfigChildY(HWND child, int y) {
    if (!child) return;
    RECT rc{};
    GetWindowRect(child, &rc);
    POINT pt{rc.left, rc.top};
    ScreenToClient(GetParent(child), &pt);
    SetWindowPos(child, nullptr, pt.x, y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SetConfigScrollOffset(ConfigState* state, int value) {
    if (!state) return;
    int tab = state->activeTab;
    int maxScroll = ConfigMaxScroll(state, tab);
    state->scrollOffset[static_cast<size_t>(tab)] = std::clamp(value, 0, maxScroll);
}

void ApplyConfigScroll(ConfigState* state) {
    if (!state) return;
    int tab = state->activeTab;
    int maxScroll = ConfigMaxScroll(state, tab);
    int offset = std::clamp(state->scrollOffset[static_cast<size_t>(tab)], 0, maxScroll);
    state->scrollOffset[static_cast<size_t>(tab)] = offset;

    if (state->scrollBar) {
        SCROLLINFO si{};
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = maxScroll + ConfigVisibleHeight() - 1;
        si.nPage = ConfigVisibleHeight();
        si.nPos = offset;
        SetScrollInfo(state->scrollBar, SB_CTL, &si, TRUE);
        ShowWindow(state->scrollBar, maxScroll > 0 ? SW_SHOW : SW_HIDE);
    }

    auto fullyVisibleAt = [](int y, int h) {
        return y >= 0 && y + h <= ConfigVisibleHeight();
    };
    for (const auto& header : state->headers) {
        int headerY = header.baseY - offset;
        bool show = header.tab == tab && fullyVisibleAt(headerY, 24);
        MoveConfigChildY(header.control, headerY);
        ShowWindow(header.control, show ? SW_SHOW : SW_HIDE);
    }
    for (const auto& field : state->fields) {
        bool active = field.def->tab == tab;
        int labelY = field.labelBaseY - offset;
        int controlY = field.controlBaseY - offset;
        int sliderY = field.sliderBaseY - offset;
        bool show = active && fullyVisibleAt(controlY, 28);
        MoveConfigChildY(field.label, labelY);
        MoveConfigChildY(field.control, controlY);
        ShowWindow(field.label, show ? SW_SHOW : SW_HIDE);
        ShowWindow(field.control, show ? SW_SHOW : SW_HIDE);
        if (field.slider) {
            MoveConfigChildY(field.slider, sliderY);
            ShowWindow(field.slider, show ? SW_SHOW : SW_HIDE);
        }
    }

    if (state->hwnd) {
        HWND target = state->scrollPane ? state->scrollPane : state->hwnd;
        RedrawWindow(target, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
    }
}

void ShowConfigTab(ConfigState* state, int tab) {
    if (!state) return;
    tab = std::clamp(tab, 0, std::max(0, state->tabCount - 1));
    state->activeTab = tab;
    if (!ConfigActiveTabIsMonsterPreview(state) && state->previewOrbitDragging) {
        EndConfigPreviewOrbit(state, GetCapture());
    }
    if (state->note && tab < static_cast<int>(state->tabNotes.size())) SetWindowTextW(state->note, state->tabNotes[static_cast<size_t>(tab)].c_str());
    ApplyConfigScroll(state);
    state->previewPending = false;
    UpdateConfigPreviewHint(state);
    if (!state->previewRenderer) {
        SetConfigPreviewStatus(state, L"Click Update preview to render current settings.");
    } else {
        SetConfigPreviewStatus(state, L"");
    }
}

