        auto fillMenuLabelAtlas = [&](int material) {
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    albedo[i + 0] = 0;
                    albedo[i + 1] = 0;
                    albedo[i + 2] = 0;
                    albedo[i + 3] = 0;
                    heights[static_cast<size_t>(gy * width + x)] = 0.50f;
                    props[i + 0] = 255;
                    props[i + 1] = 40;
                    props[i + 2] = 0;
                    props[i + 3] = 255;
                }
            }

            std::vector<uint8_t> dib(static_cast<size_t>(kTextureSize) * kTextureSize * 4, 0);
            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = kTextureSize;
            bmi.bmiHeader.biHeight = -kTextureSize;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            void* bits = nullptr;
            HDC screen = GetDC(nullptr);
            HDC dc = CreateCompatibleDC(screen);
            HBITMAP bitmap = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
            if (bitmap && bits) {
                HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
                std::memset(bits, 0, dib.size());
                SetBkMode(dc, TRANSPARENT);
                HFONT font = CreateFontW(-38, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                    DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
                HGDIOBJ oldFont = SelectObject(dc, font);
                const wchar_t* labels[] = {L"New Game", L"Resume", L"Resume Saved Run", L"Custom Game", L"Settings", L"Debug"};
                constexpr int kMenuLabelRows = 6;
                for (int row = 0; row < kMenuLabelRows; ++row) {
                    int top = row * kTextureSize / kMenuLabelRows;
                    int bottom = (row + 1) * kTextureSize / kMenuLabelRows;
                    RECT textRect{28, top + 18, kTextureSize - 28, bottom - 16};
                    SetTextColor(dc, RGB(58, 45, 18));
                    for (int oy = -3; oy <= 3; ++oy) {
                        for (int ox = -3; ox <= 3; ++ox) {
                            if (ox * ox + oy * oy > 10) continue;
                            RECT glow = textRect;
                            OffsetRect(&glow, ox, oy);
                            DrawTextW(dc, labels[row], -1, &glow, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                        }
                    }
                    SetTextColor(dc, RGB(238, 212, 132));
                    DrawTextW(dc, labels[row], -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                }
                SelectObject(dc, oldFont);
                SelectObject(dc, oldBitmap);
                DeleteObject(font);
                std::memcpy(dib.data(), bits, dib.size());
            }
            if (bitmap) DeleteObject(bitmap);
            if (dc) DeleteDC(dc);
            if (screen) ReleaseDC(nullptr, screen);

            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * kTextureSize + x) * 4);
                    uint8_t b = dib[src + 0];
                    uint8_t g = dib[src + 1];
                    uint8_t r = dib[src + 2];
                    uint8_t a = std::max(r, std::max(g, b));
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    albedo[dst + 0] = r;
                    albedo[dst + 1] = g;
                    albedo[dst + 2] = b;
                    albedo[dst + 3] = a;
                }
            }
        };
        fillMenuLabelAtlas(18);

