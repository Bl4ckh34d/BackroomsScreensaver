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
