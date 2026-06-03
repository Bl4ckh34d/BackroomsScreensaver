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
