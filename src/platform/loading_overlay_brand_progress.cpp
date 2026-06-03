#include "loading_overlay_internal.h"

void DrawLoadingBrandProgress(
    HDC hdc,
    const RECT& rc,
    const LoadingOverlaySnapshot& state,
    int width,
    int height,
    float titleAlpha,
    float doneAge,
    const LoadingBrandTitleLayout& title) {
    if (titleAlpha <= 0.18f) return;

    float detailAlpha = Clamp01((titleAlpha - 0.18f) / 0.82f) *
        (doneAge >= 0.0f ? (1.0f - SmoothStep(0.05f, 0.72f, doneAge)) : 1.0f);
    int barWidth = std::min(title.textWidth, width - 48);
    int barHeight = std::clamp(height / 58, 12, 22);
    int barLeft = rc.left + (width - barWidth) / 2;
    int barTop = title.bottom + std::clamp(height / 34, 18, 38);
    RECT barRect{barLeft, barTop, barLeft + barWidth, barTop + barHeight};
    HBRUSH barBg = CreateSolidBrush(LoadingFadeColor(32, 30, 22, detailAlpha * 0.82f));
    FillRect(hdc, &barRect, barBg);
    DeleteObject(barBg);

    int total = state.fineTotal > 0 ? std::max(1, state.fineTotal) : std::max(1, state.total);
    int current = state.fineTotal > 0 ? std::clamp(state.fineCurrent, 0, total) : state.current;
    float progress = state.complete ? 1.0f : Clamp01(static_cast<float>(current) / static_cast<float>(total));
    if (!state.complete && current > 0 && current < total) {
        ULONGLONG updateTick = state.lastUpdateTick != 0 ? state.lastUpdateTick : state.createdTick;
        float sinceUpdate = updateTick != 0
            ? static_cast<float>(GetTickCount64() - updateTick) * 0.001f
            : 0.0f;
        float stepSize = 1.0f / static_cast<float>(total);
        float intraStep = SmoothStep(0.0f, 2.25f, sinceUpdate) * stepSize * 0.72f;
        if (state.shaderTotal > 0 && state.shaderDone < state.shaderTotal) {
            intraStep = std::max(intraStep, stepSize * 0.58f *
                SmoothStep(0.0f, 1.35f, sinceUpdate));
        }
        progress = std::min(progress + intraStep,
            static_cast<float>(current + 1) / static_cast<float>(total) - stepSize * 0.12f);
    }
    RECT fillRect = barRect;
    fillRect.right = fillRect.left + static_cast<int>(std::round(static_cast<float>(barWidth) * progress));
    if (fillRect.right > fillRect.left) {
        HBRUSH barFill = CreateSolidBrush(LoadingFadeColor(kLoadingTitleR, kLoadingTitleG, kLoadingTitleB, detailAlpha));
        FillRect(hdc, &fillRect, barFill);
        DeleteObject(barFill);
    }
}
