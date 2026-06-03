// Game cursor capture and input snapshot collection.
// Included from game_shell.inl before game state transition functions.

void SetGameCursorVisible(bool visible) {
    if (visible) {
        while (ShowCursor(TRUE) < 0) {}
    } else {
        while (ShowCursor(FALSE) >= 0) {}
    }
}

void ReleaseGameMouse() {
    if (!gApp) return;
    ClipCursor(nullptr);
    if (gApp->gameMouseCaptured) {
        ReleaseCapture();
    }
    SetGameCursorVisible(true);
    gApp->gameMouseCaptured = false;
}

bool GameWindowAcceptsInput(HWND hwnd) {
    if (!gApp || hwnd == nullptr || !gApp->gameWindowActive || IsIconic(hwnd)) return false;
    HWND foreground = GetForegroundWindow();
    if (foreground == hwnd) return true;
    if (!foreground) return false;
    if (IsChild(hwnd, foreground)) return true;
    HWND root = GetAncestor(foreground, GA_ROOT);
    HWND rootOwner = GetAncestor(foreground, GA_ROOTOWNER);
    return root == hwnd || rootOwner == hwnd;
}

void CaptureGameMouse(HWND hwnd) {
    if (!gApp || !gApp->gameShell || !hwnd) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    POINT tl{rc.left, rc.top};
    POINT br{rc.right, rc.bottom};
    ClientToScreen(hwnd, &tl);
    ClientToScreen(hwnd, &br);
    RECT clip{tl.x, tl.y, br.x, br.y};
    ClipCursor(&clip);
    if (!gApp->gameMouseCaptured) {
        SetCapture(hwnd);
        SetGameCursorVisible(false);
    }
    gApp->gameMouseCaptured = true;
    gApp->gameMouseCenter = {(rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2};
    POINT center = gApp->gameMouseCenter;
    ClientToScreen(hwnd, &center);
    gApp->gameRecenteringMouse = true;
    SetCursorPos(center.x, center.y);
}

GameInputSnapshot CollectGameInput() {
    GameInputSnapshot input{};
    if (!gApp || gApp->gameState != GameState::PlayGame) return input;
    if (!GameWindowAcceptsInput(gApp->hwnd)) {
        gApp->gameMouseDeltaX = 0.0f;
        gApp->gameMouseDeltaY = 0.0f;
        return input;
    }
    auto down = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };
    const Settings& settings = gApp->gameInputSettings;
    input.moveX =
        (down(GameActionKey(settings, GameInputAction::MoveRight)) ? 1.0f : 0.0f) +
        (down(GameActionKey(settings, GameInputAction::MoveLeft)) ? -1.0f : 0.0f);
    input.moveZ =
        (down(GameActionKey(settings, GameInputAction::MoveForward)) ? 1.0f : 0.0f) +
        (down(GameActionKey(settings, GameInputAction::MoveBackward)) ? -1.0f : 0.0f);
    input.lookDeltaX = gApp->gameMouseDeltaX;
    input.lookDeltaY = gApp->gameMouseDeltaY;
    gApp->gameMouseDeltaX = 0.0f;
    gApp->gameMouseDeltaY = 0.0f;
    input.sprint = down(GameActionKey(settings, GameInputAction::Sprint));
    input.crouch = down(GameActionKey(settings, GameInputAction::Crouch));
    input.interact = down(GameActionKey(settings, GameInputAction::Interact));
    input.flashlight = down(GameActionKey(settings, GameInputAction::Flashlight));
    input.pause = down(GameActionKey(settings, GameInputAction::Pause));
    return input;
}
