        constexpr int size = kCustomMenuTextureSize;
        constexpr int logicalSize = 512;
        std::vector<uint8_t> dib(static_cast<size_t>(size) * size * 4, 0);
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = size;
        bmi.bmiHeader.biHeight = -size;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        HDC screen = GetDC(nullptr);
        HDC dc = CreateCompatibleDC(screen);
        HBITMAP bitmap = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (bitmap && bits) {
            HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
            std::memset(bits, 0xFF, dib.size());
            SetMapMode(dc, MM_ANISOTROPIC);
            SetWindowExtEx(dc, logicalSize, logicalSize, nullptr);
            SetViewportExtEx(dc, size, size, nullptr);
            SetBkMode(dc, TRANSPARENT);

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

            auto text = [&](const wchar_t* value, RECT r, HFONT font, UINT flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
                HGDIOBJ oldFont = SelectObject(dc, font);
                SetTextColor(dc, RGB(9, 9, 8));
                DrawTextW(dc, value, -1, &r, flags);
                SelectObject(dc, oldFont);
            };
            auto isHover = [&](CustomGameMenuControl control) {
                return menuRuntime_.customHoverControl == static_cast<int>(control);
            };
            auto markerLine = [&](int x0, int y0, int x1, int y1, HPEN pen) {
                HGDIOBJ previousPen = SelectObject(dc, pen);
                MoveToEx(dc, x0, y0, nullptr);
                LineTo(dc, x1, y1);
                SelectObject(dc, previousPen);
            };
            auto fillYellow = [&](RECT r, int inset = 2) {
                if (!yellowBrush) return;
                r.left += inset;
                r.top += inset;
                r.right -= inset;
                r.bottom -= inset;
                HGDIOBJ previousBrush = SelectObject(dc, yellowBrush);
                HGDIOBJ previousPen = SelectObject(dc, GetStockObject(NULL_PEN));
                RoundRect(dc, r.left, r.top, r.right, r.bottom, 8, 8);
                SelectObject(dc, previousPen);
                SelectObject(dc, previousBrush);
            };
            auto row = [&](CustomGameMenuControl control, const wchar_t* label, bool checked) {
                RECT r{};
                if (!CustomGameControlPixelRect(control, r)) return;
                RECT box{r.left + 10, r.top + 6, r.left + 26, r.top + 22};
                if (checked) fillYellow({box.left - 3, box.top - 3, box.right + 3, box.bottom + 3}, 0);
                SelectObject(dc, inkPen);
                Rectangle(dc, box.left, box.top, box.right, box.bottom);
                if (checked) {
                    HGDIOBJ previousPen = SelectObject(dc, boldInkPen);
                    MoveToEx(dc, box.left + 3, box.top + 8, nullptr);
                    LineTo(dc, box.left + 7, box.bottom - 4);
                    LineTo(dc, box.right - 3, box.top + 3);
                    SelectObject(dc, previousPen);
                }
                RECT tr{r.left + 36, r.top, r.right - 10, r.bottom};
                text(label, tr, bodyFont);
            };
            auto button = [&](CustomGameMenuControl control, const wchar_t* label) {
                RECT r{};
                if (!CustomGameControlPixelRect(control, r)) return;
                if (isHover(control)) fillYellow(r, 1);
                SelectObject(dc, isHover(control) ? hoverPen : inkPen);
                RoundRect(dc, r.left, r.top, r.right, r.bottom, 10, 10);
                text(label, r, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            };
            auto stepper = [&](CustomGameMenuControl minusControl, CustomGameMenuControl plusControl, int y, const wchar_t* label, int value) {
                RECT lr{52, y, 154, y + 28};
                text(label, lr, bodyFont);
                button(minusControl, L"-");
                RECT valueRect{204, y, 268, y + 28};
                wchar_t number[24]{};
                swprintf_s(number, L"%d", value);
                text(number, valueRect, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                button(plusControl, L"+");
            };
            auto timingStepper = [&](CustomGameMenuControl minusControl, CustomGameMenuControl plusControl, int y, const wchar_t* label, const wchar_t* value) {
                RECT lr{66, y, 172, y + 28};
                text(label, lr, bodyFont);
                button(minusControl, L"-");
                RECT valueRect{210, y, 302, y + 28};
                text(value, valueRect, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                button(plusControl, L"+");
            };
            auto detailButton = [&](CustomGameMenuControl control) {
                RECT r{};
                if (!CustomGameControlPixelRect(control, r)) return;
                if (isHover(control)) fillYellow(r, 1);
                SelectObject(dc, isHover(control) ? hoverPen : inkPen);
                RoundRect(dc, r.left + 3, r.top + 4, r.right - 3, r.bottom - 3, 7, 7);
                text(L">", r, smallFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            };
            auto finishGdiDraw = [&]() {
                SelectObject(dc, oldBrush);
                SelectObject(dc, oldPen);
                SelectObject(dc, oldBitmap);
                DeleteObject(titleFont);
                DeleteObject(bodyFont);
                DeleteObject(smallFont);
                DeleteObject(inkPen);
                DeleteObject(boldInkPen);
                DeleteObject(hoverPen);
                DeleteObject(dimInkPen);
                DeleteObject(yellowBrush);
                std::memcpy(dib.data(), bits, dib.size());
            };
