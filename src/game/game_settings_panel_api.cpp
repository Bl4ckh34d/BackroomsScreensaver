#include "game_settings_panel_internal.h"

HWND CreateGameSettingsPanel(HWND parent, GameSettingsPanelHost host) {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t* cls = L"BackroomsMazeGameSettingsPanel";
    WNDCLASSW wc{};
    wc.lpfnWndProc = GameSettingsPanelWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    return CreateWindowExW(0, cls, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 10, 10, parent, nullptr, hInstance, &host);
}
