        constexpr int paddingX = 34;
        constexpr int paddingY = 16;
        constexpr int maxTextW = 760;
        RECT measure{0, 0, maxTextW, 0};
        DrawTextW(dc, hudNotification_.text.c_str(), -1, &measure,
            DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
        bool wrap = (measure.right - measure.left) > maxTextW || hudNotification_.text.find(L'\n') != std::wstring::npos;
        if (wrap) {
            measure = {0, 0, maxTextW, 0};
            DrawTextW(dc, hudNotification_.text.c_str(), -1, &measure,
                DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
        }
        int measuredW = static_cast<int>(measure.right - measure.left);
        int measuredH = static_cast<int>(measure.bottom - measure.top);
        int texW = std::clamp(measuredW + paddingX * 2, 220, maxTextW + paddingX * 2);
        int texH = std::clamp(measuredH + paddingY * 2, 58, 160);
        hudNotification_.textureWidth = texW;
        hudNotification_.textureHeight = texH;
