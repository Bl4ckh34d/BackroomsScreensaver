#include "loading_overlay_internal.h"

void DrawLoadingBrandMark(HDC hdc, int x, int y, int logoSize, float logoAlpha) {
    const ImageRGBA& logo = LoadingOverlayLogo();
    if (!logo.Valid() || logoAlpha <= 0.002f) return;

    static std::vector<uint8_t> premultipliedLogo;
    static int cachedW = 0;
    static int cachedH = 0;
    static HBITMAP cachedBitmap = nullptr;
    static void* cachedBits = nullptr;
    if (cachedW != logo.width || cachedH != logo.height || premultipliedLogo.empty()) {
        if (cachedBitmap) {
            DeleteObject(cachedBitmap);
            cachedBitmap = nullptr;
            cachedBits = nullptr;
        }
        cachedW = logo.width;
        cachedH = logo.height;
        premultipliedLogo.assign(static_cast<size_t>(cachedW) * static_cast<size_t>(cachedH) * 4, 0);
        bool hasTransparency = false;
        for (int py = 0; py < cachedH && !hasTransparency; ++py) {
            for (int px = 0; px < cachedW; ++px) {
                size_t src = static_cast<size_t>((py * cachedW + px) * 4);
                if (logo.pixels[src + 3] < 250) {
                    hasTransparency = true;
                    break;
                }
            }
        }
        for (int py = 0; py < cachedH; ++py) {
            for (int px = 0; px < cachedW; ++px) {
                size_t src = static_cast<size_t>((py * cachedW + px) * 4);
                uint8_t r = logo.pixels[src + 0];
                uint8_t g = logo.pixels[src + 1];
                uint8_t b = logo.pixels[src + 2];
                uint8_t srcA = logo.pixels[src + 3];
                uint8_t rgbCoverage = std::max(r, std::max(g, b));
                uint8_t darkCoverage = static_cast<uint8_t>(255 - std::min(r, std::min(g, b)));
                uint8_t a = hasTransparency ? srcA : std::max(rgbCoverage, darkCoverage);
                size_t dst = src;
                premultipliedLogo[dst + 0] = static_cast<uint8_t>((static_cast<uint16_t>(kLoadingTitleB) * a) / 255u);
                premultipliedLogo[dst + 1] = static_cast<uint8_t>((static_cast<uint16_t>(kLoadingTitleG) * a) / 255u);
                premultipliedLogo[dst + 2] = static_cast<uint8_t>((static_cast<uint16_t>(kLoadingTitleR) * a) / 255u);
                premultipliedLogo[dst + 3] = a;
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
            std::memcpy(cachedBits, premultipliedLogo.data(), premultipliedLogo.size());
        }
    }
    HDC logoDc = CreateCompatibleDC(hdc);
    if (logoDc && cachedBitmap) {
        HGDIOBJ oldBitmap = SelectObject(logoDc, cachedBitmap);
        BLENDFUNCTION blend{};
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = static_cast<BYTE>(std::clamp(static_cast<int>(std::round(logoAlpha * 255.0f)), 0, 255));
        blend.AlphaFormat = AC_SRC_ALPHA;
        AlphaBlend(hdc, x, y, logoSize, logoSize, logoDc, 0, 0, cachedW, cachedH, blend);
        SelectObject(logoDc, oldBitmap);
    }
    if (logoDc) DeleteDC(logoDc);
}
