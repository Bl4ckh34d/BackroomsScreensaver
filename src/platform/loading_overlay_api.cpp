#include "loading_overlay_internal.h"

HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash) {
    if (brandedSplash) {
        if (HWND threaded = CreateThreadedLoadingOverlay(parent, hInstance, brandedSplash)) {
            return threaded;
        }
    }

    RegisterLoadingOverlayClass(hInstance);
    if (brandedSplash) {
        (void)LoadingOverlayLogo();
    }
    auto* state = new LoadingOverlayState();
    InitializeCriticalSection(&state->lock);
    state->brandedSplash = brandedSplash;
    state->threadedPopup = false;
    state->createdTick = GetTickCount64();
    state->lastUpdateTick = state->createdTick;
    RECT rc{};
    GetClientRect(parent, &rc);
    HWND overlay = CreateWindowExW(0, kLoadingOverlayClass, nullptr, WS_CHILD | WS_VISIBLE,
        0, 0, std::max(1, static_cast<int>(rc.right - rc.left)),
        std::max(1, static_cast<int>(rc.bottom - rc.top)),
        parent, nullptr, hInstance, state);
    if (!overlay) {
        DeleteCriticalSection(&state->lock);
        delete state;
        return nullptr;
    }
    SetWindowPos(overlay, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    UpdateWindow(overlay);
    return overlay;
}

void SetLoadingOverlayProgress(HWND overlay, const StartupProgressUpdate& update) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (state && !state->threadedPopup) {
        LockLoadingState(state);
        state->phase = update.phase ? update.phase : L"Loading";
        state->detail = update.detail ? update.detail : L"";
        state->current = std::clamp(update.current, 0, std::max(1, update.total));
        state->total = std::max(1, update.total);
        state->fineCurrent = std::clamp(update.fineCurrent, 0, std::max(1, update.fineTotal));
        state->fineTotal = std::max(0, update.fineTotal);
        state->shaderDone = std::clamp(update.shaderDone, 0, std::max(0, update.shaderTotal));
        state->shaderTotal = std::max(0, update.shaderTotal);
        state->shaderCompiled = std::max(0, update.shaderCompiled);
        state->shaderCached = std::max(0, update.shaderCached);
        state->lastUpdateTick = GetTickCount64();
        state->frame = (state->frame + 1) % 12;
        UnlockLoadingState(state);
        RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
        PumpLoadingOverlayMessages();
    } else {
        auto* posted = new LoadingOverlayPostedUpdate();
        posted->phase = update.phase ? update.phase : L"Loading";
        posted->detail = update.detail ? update.detail : L"";
        posted->current = update.current;
        posted->total = update.total;
        posted->fineCurrent = update.fineCurrent;
        posted->fineTotal = update.fineTotal;
        posted->shaderDone = update.shaderDone;
        posted->shaderTotal = update.shaderTotal;
        posted->shaderCompiled = update.shaderCompiled;
        posted->shaderCached = update.shaderCached;
        if (!PostMessageW(overlay, kLoadingOverlayProgressMessage, 0, reinterpret_cast<LPARAM>(posted))) {
            delete posted;
        }
    }
}

void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (state && !state->threadedPopup) {
        LockLoadingState(state);
        int total = std::max(state->total + 1, state->current + 1);
        state->phase = phase ? phase : L"Loading";
        state->detail = detail ? detail : L"";
        if (complete && !state->complete) {
            state->complete = true;
            state->completeTick = GetTickCount64();
        }
        state->total = std::max(1, total);
        state->current = complete ? state->total : std::max(0, state->total - 1);
        state->fineTotal = std::max(state->fineTotal, state->total);
        state->fineCurrent = complete ? state->fineTotal : std::min(state->fineCurrent, state->fineTotal);
        state->lastUpdateTick = GetTickCount64();
        state->frame = (state->frame + 1) % 12;
        UnlockLoadingState(state);
        RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
        PumpLoadingOverlayMessages();
    } else {
        auto* posted = new LoadingOverlayPostedUpdate();
        posted->phase = phase ? phase : L"Loading";
        posted->detail = detail ? detail : L"";
        posted->complete = complete;
        if (!PostMessageW(overlay, kLoadingOverlayStatusMessage, 0, reinterpret_cast<LPARAM>(posted))) {
            delete posted;
        }
    }
}

void CloseLoadingOverlayWindow(HWND overlay) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (state && !state->threadedPopup) {
        DestroyWindow(overlay);
        PumpLoadingOverlayMessages();
        return;
    }
    LoadingOverlayThreadInfo threadInfo{};
    auto it = gLoadingOverlayThreads.find(overlay);
    if (it != gLoadingOverlayThreads.end()) {
        threadInfo = it->second;
        gLoadingOverlayThreads.erase(it);
    }
    ShowWindowAsync(overlay, SW_HIDE);
    SetWindowPos(overlay, nullptr, 0, 0, 0, 0,
        SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    DWORD_PTR closeResult = 0;
    SendMessageTimeoutW(overlay, kLoadingOverlayCloseMessage, 0, 0,
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 700, &closeResult);
    ULONGLONG start = GetTickCount64();
    if (threadInfo.handle) {
        bool quitPosted = false;
        while (WaitForSingleObject(threadInfo.handle, 10) == WAIT_TIMEOUT && GetTickCount64() - start < 1200) {
            if (!quitPosted && GetTickCount64() - start > 140 && threadInfo.threadId != 0) {
                PostThreadMessageW(threadInfo.threadId, WM_QUIT, 0, 0);
                quitPosted = true;
            }
            PumpLoadingOverlayMessages();
        }
        CloseHandle(threadInfo.handle);
    } else {
        while (IsWindow(overlay) && GetTickCount64() - start < 1200) {
            PumpLoadingOverlayMessages();
            Sleep(10);
        }
    }
    PumpLoadingOverlayMessages();
}

void FinishLoadingOverlay(HWND overlay) {
    if (!overlay) return;
    bool branded = false;
    ULONGLONG createdTick = GetTickCount64();
    auto it = gLoadingOverlayThreads.find(overlay);
    if (it != gLoadingOverlayThreads.end()) {
        branded = it->second.brandedSplash;
        createdTick = it->second.createdTick;
    } else if (LoadingOverlayState* state = LoadingState(overlay)) {
        LockLoadingState(state);
        branded = state->brandedSplash;
        createdTick = state->createdTick;
        UnlockLoadingState(state);
    }
    if (!branded) {
        CloseLoadingOverlayWindow(overlay);
        return;
    }

    ULONGLONG completeTick = GetTickCount64();
    ULONGLONG minVisibleUntil = createdTick + kBrandedIntroReadyMs + 600;
    ULONGLONG fadeUntil = completeTick + 1050;
    ULONGLONG until = std::max(minVisibleUntil, fadeUntil);
    while (GetTickCount64() < until && IsWindow(overlay)) {
        InvalidateRect(overlay, nullptr, FALSE);
        PumpLoadingOverlayMessages();
        Sleep(16);
    }
    CloseLoadingOverlayWindow(overlay);
}

void LoadingProgressCallback(void* context, const StartupProgressUpdate& update) {
    SetLoadingOverlayProgress(static_cast<HWND>(context), update);
}
