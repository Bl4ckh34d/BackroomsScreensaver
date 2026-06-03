// Game cursor capture and input snapshot collection.
// Included from game_shell.inl before game state transition functions.

void SetGameCursorVisible(bool visible) {
    if (visible) {
        while (ShowCursor(TRUE) < 0) {}
    } else {
        while (ShowCursor(FALSE) >= 0) {}
    }
}
