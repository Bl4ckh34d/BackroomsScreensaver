#include "loading_overlay_internal.h"

DWORD WINAPI LoadingOverlayThreadProc(LPVOID param) {
    auto* start = static_cast<LoadingOverlayThreadStart*>(param);
    start->threadId = GetCurrentThreadId();
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    RegisterLoadingOverlayClass(start->hInstance);
    if (start->brandedSplash) {
        (void)LoadingOverlayLogo();
    }
    auto* state = new LoadingOverlayState();
    InitializeCriticalSection(&state->lock);
    state->brandedSplash = start->brandedSplash;
    state->threadedPopup = true;
    state->createdTick = GetTickCount64();
    state->lastUpdateTick = state->createdTick;
    RECT rc{};
    GetClientRect(start->owner, &rc);
    POINT tl{rc.left, rc.top};
    ClientToScreen(start->owner, &tl);
    int width = std::max(1, static_cast<int>(rc.right - rc.left));
    int height = std::max(1, static_cast<int>(rc.bottom - rc.top));
    HWND overlay = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kLoadingOverlayClass,
        nullptr,
        WS_POPUP | WS_VISIBLE,
        tl.x,
        tl.y,
        width,
        height,
        start->owner,
        nullptr,
        start->hInstance,
        state);
    if (!overlay) {
        DeleteCriticalSection(&state->lock);
        delete state;
        start->overlay = nullptr;
        SetEvent(start->ready);
        return 0;
    }
    start->overlay = overlay;
    SetWindowPos(overlay, HWND_TOP, tl.x, tl.y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    UpdateWindow(overlay);
    SetEvent(start->ready);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

HWND CreateThreadedLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash) {
    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!ready) return nullptr;

    LoadingOverlayThreadStart start{};
    start.owner = parent;
    start.hInstance = hInstance;
    start.ready = ready;
    start.createdTick = GetTickCount64();
    start.brandedSplash = brandedSplash;

    DWORD threadId = 0;
    HANDLE thread = CreateThread(nullptr, 0, LoadingOverlayThreadProc, &start, 0, &threadId);
    if (!thread) {
        CloseHandle(ready);
        return nullptr;
    }
    SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);

    DWORD waitResult = WaitForSingleObject(ready, 1500);
    CloseHandle(ready);
    if (waitResult != WAIT_OBJECT_0 || !start.overlay) {
        PostThreadMessageW(threadId, WM_QUIT, 0, 0);
        WaitForSingleObject(thread, 400);
        CloseHandle(thread);
        return nullptr;
    }

    LoadingOverlayThreadInfo info{};
    info.handle = thread;
    info.threadId = start.threadId != 0 ? start.threadId : threadId;
    info.brandedSplash = brandedSplash;
    info.createdTick = start.createdTick;
    if (LoadingOverlayState* state = LoadingState(start.overlay)) {
        LockLoadingState(state);
        info.createdTick = state->createdTick;
        UnlockLoadingState(state);
    }
    gLoadingOverlayThreads[start.overlay] = info;
    return start.overlay;
}

bool LoadingOverlayHasIndependentSplash(HWND overlay) {
    auto it = gLoadingOverlayThreads.find(overlay);
    return it != gLoadingOverlayThreads.end() && it->second.brandedSplash && it->second.handle;
}

void WaitForLoadingOverlayIntro(HWND overlay) {
    if (!overlay) return;
    ULONGLONG createdTick = 0;
    auto it = gLoadingOverlayThreads.find(overlay);
    if (it != gLoadingOverlayThreads.end() && it->second.brandedSplash) {
        createdTick = it->second.createdTick;
    } else if (LoadingOverlayState* state = LoadingState(overlay)) {
        LockLoadingState(state);
        if (state->brandedSplash) createdTick = state->createdTick;
        UnlockLoadingState(state);
    }
    if (createdTick == 0) return;

    ULONGLONG readyAt = createdTick + kBrandedIntroReadyMs;
    while (GetTickCount64() < readyAt && IsWindow(overlay)) {
        PumpLoadingOverlayMessages();
        Sleep(16);
    }
}
