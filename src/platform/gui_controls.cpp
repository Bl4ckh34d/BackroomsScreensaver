#include "platform_headers.h"

#include "gui_controls.h"

void ApplyDefaultGuiFont(HWND hwnd) {
    if (!hwnd) return;
    HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}
