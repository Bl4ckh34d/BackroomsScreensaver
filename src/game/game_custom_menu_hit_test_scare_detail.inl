int HitTestCustomGameMenu(HWND hwnd, POINT p) {
    if (!gApp || !gApp->gameShell || !gApp->gameCustomMenuOpen || !hwnd) return 0;
    std::vector<int> controls;
    if (gApp->gameCustomSelectedScare >= 0) {
        controls = {
            static_cast<int>(CustomGameMenuControl::ScareChanceMinus),
            static_cast<int>(CustomGameMenuControl::ScareChancePlus),
            static_cast<int>(CustomGameMenuControl::ScareStartMinMinus),
            static_cast<int>(CustomGameMenuControl::ScareStartMinPlus),
            static_cast<int>(CustomGameMenuControl::ScareStartMaxMinus),
            static_cast<int>(CustomGameMenuControl::ScareStartMaxPlus),
            static_cast<int>(CustomGameMenuControl::ScareDetailBack)
        };
