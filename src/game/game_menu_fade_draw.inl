void DrawGameMenuFade(HDC dc, const RECT& rc, float amount) {
    amount = Clamp01(amount);
    if (amount <= 0.001f) return;
    int w = std::max<LONG>(1, rc.right - rc.left);
    int h = std::max<LONG>(1, rc.bottom - rc.top);
    int alpha = std::clamp(static_cast<int>(std::round(amount * 255.0f)), 1, 255);
    HDC memDc = CreateCompatibleDC(dc);
    if (!memDc) {
        FillGameMenuRect(dc, rc, RGB(0, 0, 0));
        return;
    }
    HBITMAP bmp = CreateCompatibleBitmap(dc, w, h);
    if (!bmp) {
        DeleteDC(memDc);
        FillGameMenuRect(dc, rc, RGB(0, 0, 0));
        return;
    }
    HGDIOBJ oldBmp = SelectObject(memDc, bmp);
    RECT local{0, 0, w, h};
    FillGameMenuRect(memDc, local, RGB(0, 0, 0));
    BLENDFUNCTION blend{AC_SRC_OVER, 0, static_cast<BYTE>(alpha), 0};
    AlphaBlend(dc, rc.left, rc.top, w, h, memDc, 0, 0, w, h, blend);
    SelectObject(memDc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(memDc);
}
