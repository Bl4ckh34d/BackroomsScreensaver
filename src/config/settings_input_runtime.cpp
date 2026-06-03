#include "../platform/platform_headers.h"

#include "settings.h"

int GameActionKey(const Settings& settings, GameInputAction action) {
    size_t index = static_cast<size_t>(action);
    if (index >= settings.gameKeyBindings.size()) return 0;
    return settings.gameKeyBindings[index];
}

void SetGameActionKey(Settings& settings, GameInputAction action, int vk) {
    size_t index = static_cast<size_t>(action);
    if (index >= settings.gameKeyBindings.size()) return;
    settings.gameKeyBindings[index] = std::clamp(vk, 1, 255);
}

void AssignGameActionKey(Settings& settings, GameInputAction action, int vk) {
    vk = std::clamp(vk, 1, 255);
    int actionIndex = static_cast<int>(action);
    int previous = GameActionKey(settings, action);
    for (int i = 0; i < kGameInputActionCount; ++i) {
        if (i != actionIndex && settings.gameKeyBindings[static_cast<size_t>(i)] == vk) {
            settings.gameKeyBindings[static_cast<size_t>(i)] = previous;
            break;
        }
    }
    SetGameActionKey(settings, action, vk);
}

std::wstring KeyDisplayName(int vk) {
    vk = std::clamp(vk, 1, 255);
    if (vk >= 'A' && vk <= 'Z') return std::wstring(1, static_cast<wchar_t>(vk));
    if (vk >= '0' && vk <= '9') return std::wstring(1, static_cast<wchar_t>(vk));
    switch (vk) {
    case VK_ESCAPE: return L"Esc";
    case VK_SPACE: return L"Space";
    case VK_SHIFT: return L"Shift";
    case VK_CONTROL: return L"Ctrl";
    case VK_MENU: return L"Alt";
    case VK_TAB: return L"Tab";
    case VK_RETURN: return L"Enter";
    case VK_BACK: return L"Backspace";
    case VK_LEFT: return L"Left";
    case VK_RIGHT: return L"Right";
    case VK_UP: return L"Up";
    case VK_DOWN: return L"Down";
    case VK_PRIOR: return L"Page Up";
    case VK_NEXT: return L"Page Down";
    case VK_HOME: return L"Home";
    case VK_END: return L"End";
    case VK_INSERT: return L"Insert";
    case VK_DELETE: return L"Delete";
    case VK_LSHIFT: return L"Left Shift";
    case VK_RSHIFT: return L"Right Shift";
    case VK_LCONTROL: return L"Left Ctrl";
    case VK_RCONTROL: return L"Right Ctrl";
    case VK_LMENU: return L"Left Alt";
    case VK_RMENU: return L"Right Alt";
    default: break;
    }

    UINT scan = MapVirtualKeyW(static_cast<UINT>(vk), MAPVK_VK_TO_VSC);
    wchar_t name[96]{};
    LONG lparam = static_cast<LONG>(scan << 16);
    if (GetKeyNameTextW(lparam, name, ARRAYSIZE(name)) > 0) return name;
    wchar_t fallback[32]{};
    swprintf_s(fallback, L"VK %d", vk);
    return fallback;
}
