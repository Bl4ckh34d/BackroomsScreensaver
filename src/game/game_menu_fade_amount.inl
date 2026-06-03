float GameMenuFadeAmount(ULONGLONG now) {
    if (!gApp) return 0.0f;
    constexpr float kFadeInMs = 1350.0f;
    constexpr float kFadeOutMs = 950.0f;
    if (gApp->gameMenuFadeOut) {
        return Clamp01(static_cast<float>(now - gApp->gameMenuFadeStart) / kFadeOutMs);
    }
    if (gApp->gameMenuFadeIn) {
        return 1.0f - Clamp01(static_cast<float>(now - gApp->gameMenuFadeStart) / kFadeInMs);
    }
    return 0.0f;
}
