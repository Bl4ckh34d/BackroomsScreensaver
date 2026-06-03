    for (int control : controls) {
        RECT rc{};
        if (!gApp->renderer.CustomGameControlScreenRect(control, rc)) continue;
        if (p.x >= rc.left && p.x <= rc.right && p.y >= rc.top && p.y <= rc.bottom) return control;
    }
    return 0;
}
