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
