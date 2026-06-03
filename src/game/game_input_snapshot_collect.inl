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
    for (int vk = VK_BACK; vk <= VK_OEM_CLEAR; ++vk) {
        if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON ||
            vk == VK_XBUTTON1 || vk == VK_XBUTTON2) {
            continue;
        }
        if (down(vk)) {
            input.anyKey = true;
            break;
        }
    }
    return input;
}
