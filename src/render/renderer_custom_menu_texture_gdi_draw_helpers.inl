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
