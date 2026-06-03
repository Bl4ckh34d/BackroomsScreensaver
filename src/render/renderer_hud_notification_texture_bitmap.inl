        std::vector<uint8_t> pixels(static_cast<size_t>(texW) * static_cast<size_t>(texH) * 4, 0);
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = texW;
        bmi.bmiHeader.biHeight = -texH;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!bmp || !bits) {
            SelectObject(dc, oldFont);
            DeleteObject(font);
            DeleteDC(dc);
            return false;
        }
        HGDIOBJ oldBmp = SelectObject(dc, bmp);
        std::memset(bits, 0, pixels.size());

        RECT textRect{paddingX, paddingY / 2, texW - paddingX, texH - paddingY / 2};
        UINT textFlags = wrap
            ? (DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS)
            : (DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        DrawTextW(dc, hudNotification_.text.c_str(), -1, &textRect, textFlags);
        SelectObject(dc, oldFont);
        DeleteObject(font);
        SelectObject(dc, oldBmp);

        const uint8_t* src = static_cast<const uint8_t*>(bits);
        for (int y = 0; y < texH; ++y) {
            for (int x = 0; x < texW; ++x) {
                size_t i = static_cast<size_t>((y * texW + x) * 4);
                uint8_t a = std::max(src[i + 0], std::max(src[i + 1], src[i + 2]));
                if (a == 0) continue;
                pixels[i + 0] = 218;
                pixels[i + 1] = 230;
                pixels[i + 2] = 246;
                pixels[i + 3] = a;
            }
        }
        DeleteObject(bmp);
        DeleteDC(dc);
