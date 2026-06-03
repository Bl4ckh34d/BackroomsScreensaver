// Config dialog preview orbit.

bool ConfigActiveTabIsMonsterPreview(const ConfigState* state) {
    return state && ((state->mode == ConfigDialogMode::Full && state->activeTab == 7) ||
        (state->mode == ConfigDialogMode::Debug && state->activeTab == 3));
}

Settings SettingsFromConfigControls(const ConfigState* state);

void ApplyConfigPreviewOrbit(ConfigState* state) {
    if (!state || !state->previewRenderer || !ConfigActiveTabIsMonsterPreview(state)) return;
    state->previewRenderer->SetMonsterPreviewOrbit(
        state->previewOrbitYaw,
        state->previewOrbitPitch,
        state->previewOrbitDistance);
}

POINT ConfigClientPointToScreen(HWND hwnd, LPARAM lParam) {
    POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ClientToScreen(hwnd, &p);
    return p;
}

bool ConfigPointOverPreview(ConfigState* state, POINT screenPoint) {
    if (!state || !state->preview || !ConfigActiveTabIsMonsterPreview(state)) return false;
    RECT rc{};
    if (!GetWindowRect(state->preview, &rc)) return false;
    return PtInRect(&rc, screenPoint) != FALSE;
}

void BeginConfigPreviewOrbit(ConfigState* state, HWND captureWindow, POINT screenPoint) {
    if (!state || !ConfigActiveTabIsMonsterPreview(state)) return;
    if (state->previewStatus) ShowWindow(state->previewStatus, SW_HIDE);
    SetFocus(state->preview ? state->preview : captureWindow);
    SetCapture(captureWindow);
    state->previewOrbitDragging = true;
    state->previewManualOrbit = true;
    state->previewLastMouse = screenPoint;
    ApplyConfigPreviewOrbit(state);
}

void EndConfigPreviewOrbit(ConfigState* state, HWND captureWindow) {
    if (!state || !state->previewOrbitDragging) return;
    state->previewOrbitDragging = false;
    if (captureWindow && GetCapture() == captureWindow) ReleaseCapture();
}

void UpdateConfigPreviewOrbit(ConfigState* state, POINT screenPoint) {
    if (!state || !state->previewOrbitDragging || !ConfigActiveTabIsMonsterPreview(state)) return;
    int dx = screenPoint.x - state->previewLastMouse.x;
    int dy = screenPoint.y - state->previewLastMouse.y;
    state->previewLastMouse = screenPoint;
    state->previewOrbitYaw += static_cast<float>(dx) * 0.010f;
    state->previewOrbitPitch = std::clamp(
        state->previewOrbitPitch + static_cast<float>(dy) * 0.0075f,
        -1.15f,
        0.75f);
    state->previewManualOrbit = true;
    ApplyConfigPreviewOrbit(state);
}

void ZoomConfigPreviewOrbit(ConfigState* state, WPARAM wParam) {
    if (!state || !ConfigActiveTabIsMonsterPreview(state) || !state->previewRenderer) return;
    int detents = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    if (detents == 0) return;
    float factor = detents > 0 ? 0.86f : 1.16f;
    int steps = std::max(1, std::abs(detents));
    for (int i = 0; i < steps; ++i) state->previewOrbitDistance *= factor;
    state->previewOrbitDistance = std::clamp(state->previewOrbitDistance, 1.25f, 8.0f);
    state->previewManualOrbit = true;
    ApplyConfigPreviewOrbit(state);
}
