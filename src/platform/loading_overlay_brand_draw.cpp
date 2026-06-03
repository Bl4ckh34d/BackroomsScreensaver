#include "loading_overlay_internal.h"

void DrawTintedLoadingLogo(HDC hdc, const RECT& rc, const LoadingOverlaySnapshot* state) {
    int width = std::max<LONG>(1, rc.right - rc.left);
    int height = std::max<LONG>(1, rc.bottom - rc.top);
    int logoSize = std::clamp(static_cast<int>(std::min(width, height) * 0.46f), 240, 430);
    int x = rc.left + (width - logoSize) / 2;
    int y = rc.top + height / 2 - logoSize / 2 - std::clamp(height / 24, 18, 42);

    float age = 0.0f;
    float doneAge = -1.0f;
    if (state && state->createdTick != 0) {
        age = static_cast<float>(GetTickCount64() - state->createdTick) * 0.001f;
        if (state->complete && state->completeTick != 0) {
            doneAge = static_cast<float>(GetTickCount64() - state->completeTick) * 0.001f;
        }
    }

    float logoAlpha = SmoothStep(0.45f, 1.85f, age) * (1.0f - SmoothStep(3.40f, 4.45f, age));
    float titleAlpha = SmoothStep(4.85f, 6.65f, age);
    if (doneAge >= 0.0f) {
        titleAlpha *= 1.0f - SmoothStep(0.10f, 0.95f, doneAge);
    }

    DrawLoadingBrandMark(hdc, x, y, logoSize, logoAlpha);
    SetBkMode(hdc, TRANSPARENT);
    LoadingBrandTitleLayout title = DrawLoadingBrandTitle(hdc, rc, width, height, titleAlpha);
    if (state) {
        DrawLoadingBrandProgress(hdc, rc, *state, width, height, titleAlpha, doneAge, title);
    }
    if (state && state->brandedSplash) return;

    DrawLoadingBrandFallbackDetail(hdc, rc, state, x, y, logoSize);
}
