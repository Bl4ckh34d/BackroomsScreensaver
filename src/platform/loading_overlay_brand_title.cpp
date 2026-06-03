#include "loading_overlay_internal.h"

LoadingBrandTitleLayout DrawLoadingBrandTitle(HDC hdc, const RECT& rc, int width, int height, float titleAlpha) {
    LoadingBrandTitleLayout result{};
    result.textWidth = std::clamp(width * 34 / 100, 220, 520);

    int titleSize = std::clamp(height / 9, 62, 138);
    result.bottom = rc.top + height / 2 + titleSize / 2;
    if (titleAlpha <= 0.002f) return result;

    const ImageRGBA& titleLogo = LoadingOverlayTitleLogo();
    if (titleLogo.Valid()) {
        static std::vector<uint8_t> premultipliedTitle;
        static int cachedW = 0;
        static int cachedH = 0;
        static HBITMAP cachedBitmap = nullptr;
        static void* cachedBits = nullptr;
        if (cachedW != titleLogo.width || cachedH != titleLogo.height || premultipliedTitle.empty()) {
            if (cachedBitmap) {
                DeleteObject(cachedBitmap);
                cachedBitmap = nullptr;
                cachedBits = nullptr;
            }
            cachedW = titleLogo.width;
            cachedH = titleLogo.height;
            premultipliedTitle.assign(static_cast<size_t>(cachedW) * static_cast<size_t>(cachedH) * 4, 0);
            for (int py = 0; py < cachedH; ++py) {
                for (int px = 0; px < cachedW; ++px) {
                    size_t src = static_cast<size_t>((py * cachedW + px) * 4);
                    uint8_t r = titleLogo.pixels[src + 0];
                    uint8_t g = titleLogo.pixels[src + 1];
                    uint8_t b = titleLogo.pixels[src + 2];
                    uint8_t a = titleLogo.pixels[src + 3];
                    premultipliedTitle[src + 0] = static_cast<uint8_t>((static_cast<uint16_t>(b) * a) / 255u);
                    premultipliedTitle[src + 1] = static_cast<uint8_t>((static_cast<uint16_t>(g) * a) / 255u);
                    premultipliedTitle[src + 2] = static_cast<uint8_t>((static_cast<uint16_t>(r) * a) / 255u);
                    premultipliedTitle[src + 3] = a;
                }
            }
        }
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = cachedW;
        bmi.bmiHeader.biHeight = -cachedH;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        SetStretchBltMode(hdc, HALFTONE);
        if (!cachedBitmap) {
            cachedBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &cachedBits, nullptr, 0);
            if (cachedBits) {
                std::memcpy(cachedBits, premultipliedTitle.data(), premultipliedTitle.size());
            }
        }
        float aspect = static_cast<float>(std::max(1, cachedW)) / static_cast<float>(std::max(1, cachedH));
        int maxTitleW = std::clamp(width * 92 / 100, 260, std::max(260, width - 32));
        int maxTitleH = std::clamp(height / 2, 120, 520);
        int drawW = std::min(maxTitleW, static_cast<int>(std::round(static_cast<float>(maxTitleH) * aspect)));
        int drawH = std::max(1, static_cast<int>(std::round(static_cast<float>(drawW) / std::max(0.001f, aspect))));
        if (drawH > maxTitleH) {
            drawH = maxTitleH;
            drawW = std::max(1, static_cast<int>(std::round(static_cast<float>(drawH) * aspect)));
        }
        int titleX = rc.left + (width - drawW) / 2;
        int titleY = rc.top + height / 2 - drawH / 2;
        HDC titleDc = CreateCompatibleDC(hdc);
        if (titleDc && cachedBitmap) {
            HGDIOBJ oldBitmap = SelectObject(titleDc, cachedBitmap);
            BLENDFUNCTION blend{};
            blend.BlendOp = AC_SRC_OVER;
            blend.SourceConstantAlpha = static_cast<BYTE>(std::clamp(static_cast<int>(std::round(titleAlpha * 255.0f)), 0, 255));
            blend.AlphaFormat = AC_SRC_ALPHA;
            AlphaBlend(hdc, titleX, titleY, drawW, drawH, titleDc, 0, 0, cachedW, cachedH, blend);
            SelectObject(titleDc, oldBitmap);
        }
        if (titleDc) DeleteDC(titleDc);
        result.textWidth = drawW;
        result.bottom = titleY + drawH;
        return result;
    }

    HFONT titleFont = CreateFontW(-titleSize, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Arial");
    HGDIOBJ oldFont = SelectObject(hdc, titleFont);
    SetTextColor(hdc, LoadingFadeColor(kLoadingTitleR, kLoadingTitleG, kLoadingTitleB, titleAlpha));
    RECT titleRect{rc.left + 24, rc.top + height / 2 - titleSize, rc.right - 24, rc.top + height / 2 + titleSize};
    SIZE textExtent{};
    if (GetTextExtentPoint32W(hdc, L"BACKROOMS", 9, &textExtent) && textExtent.cx > 0) {
        result.textWidth = std::clamp(static_cast<int>(textExtent.cx), 180, std::max(180, width - 48));
    }
    DrawTextW(hdc, L"BACKROOMS", -1, &titleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);
    return result;
}
