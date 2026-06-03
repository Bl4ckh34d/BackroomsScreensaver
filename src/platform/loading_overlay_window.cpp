#include "loading_overlay_internal.h"

LRESULT CALLBACK LoadingOverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCCREATE: {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return TRUE;
    }
    case WM_CREATE:
        SetTimer(hwnd, kLoadingOverlayTimerId, 16, nullptr);
        return 0;
    case WM_TIMER:
        if (wParam == kLoadingOverlayTimerId) {
            if (LoadingOverlayState* state = LoadingState(hwnd)) {
                LockLoadingState(state);
                state->frame = (state->frame + 1) % 12;
                UnlockLoadingState(state);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_SETCURSOR:
        SetCursor(nullptr);
        return TRUE;
    default:
        if (msg == kLoadingOverlayCloseMessage) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (msg == kLoadingOverlayProgressMessage || msg == kLoadingOverlayStatusMessage) {
            std::unique_ptr<LoadingOverlayPostedUpdate> update(reinterpret_cast<LoadingOverlayPostedUpdate*>(lParam));
            if (update) {
                if (LoadingOverlayState* state = LoadingState(hwnd)) {
                    LockLoadingState(state);
                    state->phase = update->phase.empty() ? L"Loading" : update->phase;
                    state->detail = update->detail;
                    if (msg == kLoadingOverlayProgressMessage) {
                        state->current = std::clamp(update->current, 0, std::max(1, update->total));
                        state->total = std::max(1, update->total);
                        state->fineCurrent = std::clamp(update->fineCurrent, 0, std::max(1, update->fineTotal));
                        state->fineTotal = std::max(0, update->fineTotal);
                        state->shaderDone = std::clamp(update->shaderDone, 0, std::max(0, update->shaderTotal));
                        state->shaderTotal = std::max(0, update->shaderTotal);
                        state->shaderCompiled = std::max(0, update->shaderCompiled);
                        state->shaderCached = std::max(0, update->shaderCached);
                    } else {
                        int total = std::max(state->total + 1, state->current + 1);
                        if (update->complete && !state->complete) {
                            state->complete = true;
                            state->completeTick = GetTickCount64();
                        }
                        state->total = std::max(1, total);
                        state->current = update->complete ? state->total : std::max(0, state->total - 1);
                        state->fineTotal = std::max(state->fineTotal, state->total);
                        state->fineCurrent = update->complete ? state->fineTotal : std::min(state->fineCurrent, state->fineTotal);
                    }
                    state->lastUpdateTick = GetTickCount64();
                    state->frame = (state->frame + 1) % 12;
                    UnlockLoadingState(state);
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);
        PaintLoadingOverlayBuffered(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_NCDESTROY:
        KillTimer(hwnd, kLoadingOverlayTimerId);
        if (LoadingOverlayState* state = LoadingState(hwnd)) {
            bool shouldQuit = state->threadedPopup;
            DeleteCriticalSection(&state->lock);
            delete state;
            if (shouldQuit) PostQuitMessage(0);
        }
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void RegisterLoadingOverlayClass(HINSTANCE hInstance) {
    static bool registered = false;
    if (registered) return;
    WNDCLASSW wc{};
    wc.lpfnWndProc = LoadingOverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kLoadingOverlayClass;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.hCursor = nullptr;
    RegisterClassW(&wc);
    registered = true;
}

void ResizeLoadingOverlay(HWND parent, HWND overlay) {
    if (!parent || !overlay) return;
    RECT rc{};
    GetClientRect(parent, &rc);
    LoadingOverlayState* state = LoadingState(overlay);
    bool popup = false;
    if (state) {
        LockLoadingState(state);
        popup = state->threadedPopup;
        UnlockLoadingState(state);
    }
    if (popup) {
        POINT tl{rc.left, rc.top};
        ClientToScreen(parent, &tl);
        SetWindowPos(overlay, HWND_TOP, tl.x, tl.y,
            std::max(1, static_cast<int>(rc.right - rc.left)),
            std::max(1, static_cast<int>(rc.bottom - rc.top)),
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
    } else {
        MoveWindow(overlay, 0, 0,
            std::max(1, static_cast<int>(rc.right - rc.left)),
            std::max(1, static_cast<int>(rc.bottom - rc.top)), TRUE);
    }
}

void PumpLoadingOverlayMessages() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}
