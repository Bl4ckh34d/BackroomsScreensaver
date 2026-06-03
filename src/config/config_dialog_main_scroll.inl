static bool HandleConfigVScroll(ConfigState* state, WPARAM wParam, LPARAM lParam) {
    if (state && reinterpret_cast<HWND>(lParam) == state->scrollBar) {
        int offset = state->scrollOffset[static_cast<size_t>(state->activeTab)];
        int maxScroll = ConfigMaxScroll(state, state->activeTab);
        switch (LOWORD(wParam)) {
        case SB_LINEUP: offset -= kConfigScrollStep; break;
        case SB_LINEDOWN: offset += kConfigScrollStep; break;
        case SB_PAGEUP: offset -= ConfigVisibleHeight() - kConfigScrollStep; break;
        case SB_PAGEDOWN: offset += ConfigVisibleHeight() - kConfigScrollStep; break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: {
            SCROLLINFO si{};
            si.cbSize = sizeof(si);
            si.fMask = SIF_TRACKPOS;
            GetScrollInfo(state->scrollBar, SB_CTL, &si);
            offset = si.nTrackPos;
            break;
        }
        case SB_TOP: offset = 0; break;
        case SB_BOTTOM: offset = maxScroll; break;
        default: break;
        }
        SetConfigScrollOffset(state, offset);
        ApplyConfigScroll(state);
        return true;
    }
    return false;
}

static bool HandleConfigMouseWheel(ConfigState* state, WPARAM wParam, LPARAM lParam) {
    if (state && ConfigActiveTabIsMonsterPreview(state) && state->previewRenderer) {
        POINT screenPoint{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ConfigPointOverPreview(state, screenPoint)) {
            ZoomConfigPreviewOrbit(state, wParam);
            return true;
        }
    }
    if (state && ConfigMaxScroll(state, state->activeTab) > 0) {
        int detents = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        int offset = state->scrollOffset[static_cast<size_t>(state->activeTab)] - detents * kConfigScrollStep;
        SetConfigScrollOffset(state, offset);
        ApplyConfigScroll(state);
        return true;
    }
    return false;
}
