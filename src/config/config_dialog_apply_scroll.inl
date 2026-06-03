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
