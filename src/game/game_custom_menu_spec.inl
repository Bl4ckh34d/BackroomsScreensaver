// Custom-game menu spec.

int CustomGameEditInt(HWND edit, int fallback) {
    if (!edit) return fallback;
    wchar_t text[32]{};
    GetWindowTextW(edit, text, static_cast<int>(std::size(text)));
    wchar_t* end = nullptr;
    long value = std::wcstol(text, &end, 10);
    return end != text ? static_cast<int>(value) : fallback;
}

bool CustomGameChecked(HWND control) {
    return control && SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

CustomGameSpec ReadCustomGameSpecFromControls() {
    CustomGameSpec spec = gApp ? gApp->gameCustomSpec : CustomGameSpec{};
    spec.layer = 1;
    spec.mazeWidth = std::clamp(spec.mazeWidth | 1, 3, 151);
    spec.mazeHeight = std::clamp(spec.mazeHeight | 1, 3, 151);
    spec.roomCount = std::clamp(spec.roomCount, 0, 80);
    spec.mapDirtPercent = std::clamp(spec.mapDirtPercent, 0, 100);
    spec.paperDensityPercent = std::clamp(spec.paperDensityPercent, 0, 400);
    spec.propDensityPercent = std::clamp(spec.propDensityPercent, 0, 400);
    spec.lampOnPercent = std::clamp(spec.lampOnPercent, 0, 100);
    spec.lampFlickerPercent = std::clamp(spec.lampFlickerPercent, 0, 100);
    spec.lampSparkPercent = std::clamp(spec.lampSparkPercent, 0, 100);
    spec.fogStartMeters = std::clamp(spec.fogStartMeters, 0, 200);
    spec.fogEndMeters = std::clamp(spec.fogEndMeters, spec.fogStartMeters + 1, 300);
    spec.fogDarknessPercent = std::clamp(spec.fogDarknessPercent, 0, 100);
    spec.jumpscareChancePercent = std::clamp(spec.jumpscareChancePercent, 0, 100);
    spec.jumpscareStartMinSeconds = std::clamp(spec.jumpscareStartMinSeconds, 0, 600);
    spec.jumpscareStartMaxSeconds = std::clamp(spec.jumpscareStartMaxSeconds, spec.jumpscareStartMinSeconds, 600);
    for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
        spec.scareChancePercent[i] = std::clamp(spec.scareChancePercent[i], 0, 100);
        spec.scareStartMinSeconds[i] = std::clamp(spec.scareStartMinSeconds[i], 0, 600);
        spec.scareStartMaxSeconds[i] = std::clamp(spec.scareStartMaxSeconds[i], spec.scareStartMinSeconds[i], 600);
    }
    return spec;
}
