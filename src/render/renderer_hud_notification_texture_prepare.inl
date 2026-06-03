
        if (!hudNotification_.textureDirty) return hudNotification_.srv != nullptr;
        hudNotification_.textureDirty = false;
        hudNotification_.texture.Reset();
        hudNotification_.srv.Reset();
        if (hudNotification_.text.empty() || !d3dRuntime_.device) return false;

        HDC dc = CreateCompatibleDC(nullptr);
        if (!dc) {
            if (dc) DeleteDC(dc);
            return false;
        }
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));
        const int fontPx = 34;
        HFONT font = CreateFontW(-fontPx, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(dc, font);
