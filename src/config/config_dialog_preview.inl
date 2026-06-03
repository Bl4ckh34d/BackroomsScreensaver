// Config dialog preview renderer and monster orbit helpers.

void SetConfigPreviewStatus(ConfigState* state, const wchar_t* text) {
    if (!state || !state->previewStatus) return;
    if (text && text[0] != L'\0') {
        SetWindowTextW(state->previewStatus, text);
        ShowWindow(state->previewStatus, SW_SHOW);
    } else {
        ShowWindow(state->previewStatus, SW_HIDE);
    }
}

void UpdateConfigPreviewHint(ConfigState* state) {
    if (!state || !state->previewHint) return;
    const wchar_t* hint = ConfigActiveTabIsMonsterPreview(state)
        ? L"Click Update preview after changing monster settings. Drag the preview to rotate the skull; use the wheel to zoom."
        : L"Edit several values, then click Update preview to rebuild the embedded screensaver once.";
    SetWindowTextW(state->previewHint, hint);
}

void MarkConfigPreviewDirty(ConfigState* state) {
    if (!state || state->loadingControls) return;
    state->previewPending = false;
    SetConfigPreviewStatus(state, state->previewRenderer
        ? L"Preview settings changed. Click Update preview."
        : L"Click Update preview to render current settings.");
}

void RestartConfigPreview(ConfigState* state) {
    if (!state || !state->preview) return;
    SetConfigPreviewStatus(state, L"Updating preview...");
    if (state->previewStatus) UpdateWindow(state->previewStatus);
    state->previewRenderer.reset();
    state->previewPending = false;
    Settings previewSettings = SettingsFromConfigControls(state);
    state->previewRenderer = std::make_unique<Renderer>();
    bool monsterPreview = ConfigActiveTabIsMonsterPreview(state);
    if (monsterPreview && !previewSettings.monsterAltSkullMesh.empty()) {
        previewSettings.monsterAltSkullChance = 1.0f;
    }
    MonsterPreviewView previewView = monsterPreview ? MonsterPreviewView::Front : MonsterPreviewView::Orbit;
    if (state->previewRenderer->Initialize(state->preview, &previewSettings, monsterPreview, previewView)) {
        if (monsterPreview) ApplyConfigPreviewOrbit(state);
        SetConfigPreviewStatus(state, L"");
    } else {
        state->previewRenderer.reset();
        SetConfigPreviewStatus(state, L"Direct3D preview unavailable.");
    }
}

void ScheduleConfigPreview(ConfigState* state, ULONGLONG delayMs = 450) {
    (void)delayMs;
    MarkConfigPreviewDirty(state);
}

