#include "loading_overlay_internal.h"

std::unordered_map<HWND, LoadingOverlayThreadInfo> gLoadingOverlayThreads;
uint8_t LoadingLerpChannel(uint8_t a, uint8_t b, float t) {
    return static_cast<uint8_t>(std::clamp(Lerp(static_cast<float>(a), static_cast<float>(b), Clamp01(t)), 0.0f, 255.0f));
}

COLORREF LoadingFadeColor(uint8_t r, uint8_t g, uint8_t b, float alpha) {
    return RGB(
        LoadingLerpChannel(kLoadingBgR, r, alpha),
        LoadingLerpChannel(kLoadingBgG, g, alpha),
        LoadingLerpChannel(kLoadingBgB, b, alpha));
}

LoadingOverlayState* LoadingState(HWND hwnd) {
    return reinterpret_cast<LoadingOverlayState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

void LockLoadingState(LoadingOverlayState* state) {
    if (state) EnterCriticalSection(&state->lock);
}

void UnlockLoadingState(LoadingOverlayState* state) {
    if (state) LeaveCriticalSection(&state->lock);
}

LoadingOverlaySnapshot CaptureLoadingOverlaySnapshot(HWND hwnd) {
    LoadingOverlaySnapshot snap{};
    if (LoadingOverlayState* state = LoadingState(hwnd)) {
        LockLoadingState(state);
        snap.phase = state->phase;
        snap.detail = state->detail;
        snap.brandedSplash = state->brandedSplash;
        snap.current = state->current;
        snap.total = state->total;
        snap.fineCurrent = state->fineCurrent;
        snap.fineTotal = state->fineTotal;
        snap.shaderDone = state->shaderDone;
        snap.shaderTotal = state->shaderTotal;
        snap.shaderCompiled = state->shaderCompiled;
        snap.shaderCached = state->shaderCached;
        snap.frame = state->frame;
        snap.createdTick = state->createdTick;
        snap.lastUpdateTick = state->lastUpdateTick;
        snap.completeTick = state->completeTick;
        snap.complete = state->complete;
        UnlockLoadingState(state);
    }
    return snap;
}
