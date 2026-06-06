void ReleaseGameMouse() {
    if (!gApp) return;
    UnregisterGameRawMouse();
    ClipCursor(nullptr);
    if (gApp->gameMouseCaptured) {
        ReleaseCapture();
    }
    SetGameCursorVisible(true);
    gApp->gameMouseCaptured = false;
}
