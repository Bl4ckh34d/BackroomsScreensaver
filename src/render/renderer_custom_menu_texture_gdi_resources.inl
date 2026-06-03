            HPEN inkPen = CreatePen(PS_SOLID, 2, RGB(12, 12, 10));
            HPEN boldInkPen = CreatePen(PS_SOLID, 3, RGB(5, 5, 4));
            HPEN hoverPen = CreatePen(PS_SOLID, 4, RGB(4, 4, 3));
            HPEN dimInkPen = CreatePen(PS_SOLID, 1, RGB(62, 62, 55));
            HBRUSH yellowBrush = CreateSolidBrush(RGB(246, 220, 64));
            HGDIOBJ oldPen = SelectObject(dc, inkPen);
            HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));

            HFONT titleFont = CreateFontW(-31, 0, -20, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SCRIPT, L"Segoe Print");
            HFONT bodyFont = CreateFontW(-20, 0, -10, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SCRIPT, L"Segoe Print");
            HFONT smallFont = CreateFontW(-16, 0, -8, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SCRIPT, L"Segoe Print");
