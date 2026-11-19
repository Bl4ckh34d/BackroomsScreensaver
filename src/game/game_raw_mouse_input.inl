bool RegisterGameRawMouse(HWND hwnd) {
    if (!gApp || !hwnd) return false;
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = 0;
    rid.hwndTarget = hwnd;
    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) return false;
    gApp->gameRawMouseRegistered = true;
    return true;
}

void UnregisterGameRawMouse() {
    if (!gApp || !gApp->gameRawMouseRegistered) return;
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_REMOVE;
    rid.hwndTarget = nullptr;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
    gApp->gameRawMouseRegistered = false;
}

bool HandleGameRawInput(HWND hwnd, LPARAM lParam) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd || gApp->gameState != GameState::PlayGame) {
        return false;
    }
    if (!GameWindowAcceptsInput(hwnd)) return true;

    RAWINPUT raw{};
    UINT size = sizeof(raw);
    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) != sizeof(raw)) {
        return true;
    }

    if (raw.header.dwType != RIM_TYPEMOUSE) return true;
    const RAWMOUSE& mouse = raw.data.mouse;
    if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) != 0) return true;
    gApp->gameMouseDeltaX += static_cast<float>(mouse.lLastX);
    gApp->gameMouseDeltaY += static_cast<float>(mouse.lLastY);
    return true;
}
