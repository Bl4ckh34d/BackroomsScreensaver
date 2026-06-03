int GameMenuHoverButtonIndex(int hoverId) {
    const auto buttons = ActiveGameMenuButtons();
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        if (hoverId == buttons[static_cast<size_t>(i)].id) return i;
    }
    return -1;
}

bool GameMenuHoverIsButton(int hoverId) {
    return GameMenuHoverButtonIndex(hoverId) >= 0;
}
