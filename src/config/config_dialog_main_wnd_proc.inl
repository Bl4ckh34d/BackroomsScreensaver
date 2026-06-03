#include "config_dialog_main_create.inl"
#include "config_dialog_main_notify_timer.inl"
#include "config_dialog_main_scroll.inl"
#include "config_dialog_main_preview_input.inl"
#include "config_dialog_main_commands.inl"
#include "config_dialog_main_lifecycle.inl"

// Config dialog main window procedure.

LRESULT CALLBACK ConfigWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ConfigState* state = reinterpret_cast<ConfigState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_CREATE:
        return OnConfigCreate(hwnd, lParam);
    case WM_NOTIFY:
        HandleConfigNotify(state, lParam);
        return 0;
    case WM_TIMER:
        if (HandleConfigPreviewTimer(state, wParam)) return 0;
        break;
    case WM_VSCROLL:
        if (HandleConfigVScroll(state, wParam, lParam)) return 0;
        break;
    case WM_MOUSEWHEEL:
        if (HandleConfigMouseWheel(state, wParam, lParam)) return 0;
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (HandleConfigPreviewButtonDown(state, hwnd, lParam)) return 0;
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        if (HandleConfigPreviewButtonUp(state, hwnd)) return 0;
        break;
    case WM_CAPTURECHANGED:
        HandleConfigCaptureChanged(state, hwnd, lParam);
        break;
    case WM_MOUSEMOVE:
        if (HandleConfigPreviewMouseMove(state, hwnd, lParam)) return 0;
        break;
    case WM_COMMAND:
        if (HandleConfigCommand(hwnd, state, wParam)) return 0;
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        HandleConfigDestroy(hwnd, state);
        return 0;
    case WM_NCDESTROY:
        HandleConfigNcDestroy(hwnd, state);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
