#include "loading_overlay_internal.h"

void DrawLoadingOverlay(HWND hwnd, HDC hdc) {
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int width = std::max(1, static_cast<int>(rc.right - rc.left));
    int height = std::max(1, static_cast<int>(rc.bottom - rc.top));
    LoadingOverlaySnapshot snap = CaptureLoadingOverlaySnapshot(hwnd);
    const LoadingOverlaySnapshot* state = &snap;

    HBRUSH bg = CreateSolidBrush(RGB(kLoadingBgR, kLoadingBgG, kLoadingBgB));
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);

    if (state && state->brandedSplash) {
        DrawTintedLoadingLogo(hdc, rc, state);
        return;
    }

    int contentW = std::min(560, std::max(180, width - 48));
    int left = (width - contentW) / 2;
    int top = std::max(18, height / 2 - 82);
    int spinnerRadius = std::clamp(std::min(width, height) / 18, 12, 24);
    int spinnerCx = left + spinnerRadius + 8;
    int spinnerCy = top + spinnerRadius + 4;
    DrawLoadingSpinner(hdc, spinnerCx, spinnerCy, spinnerRadius, state ? state->frame : 0);

    SetBkMode(hdc, TRANSPARENT);
    HFONT titleFont = CreateFontW(-22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT detailFont = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    int textLeft = spinnerCx + spinnerRadius + 18;
    RECT titleRect{textLeft, top - 1, left + contentW, top + 31};
    SetTextColor(hdc, RGB(245, 235, 184));
    HGDIOBJ oldFont = SelectObject(hdc, titleFont);
    const std::wstring title = state && !state->phase.empty() ? state->phase : L"Loading";
    DrawTextW(hdc, title.c_str(), -1, &titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    RECT detailRect{textLeft, top + 31, left + contentW, top + 58};
    SelectObject(hdc, detailFont);
    SetTextColor(hdc, RGB(184, 178, 137));
    const std::wstring detail = state && !state->detail.empty() ? state->detail : L"Preparing startup tasks.";
    DrawTextW(hdc, detail.c_str(), -1, &detailRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    int barTop = top + 75;
    RECT barRect{left, barTop, left + contentW, barTop + 14};
    HBRUSH barBg = CreateSolidBrush(RGB(34, 34, 27));
    FillRect(hdc, &barRect, barBg);
    DeleteObject(barBg);
    FrameRect(hdc, &barRect, reinterpret_cast<HBRUSH>(GetStockObject(GRAY_BRUSH)));

    int current = state ? state->current : 0;
    int total = std::max(1, state ? state->total : 1);
    float fraction = Clamp01(static_cast<float>(current) / static_cast<float>(total));
    RECT fillRect = barRect;
    fillRect.right = fillRect.left + static_cast<int>((barRect.right - barRect.left) * fraction);
    InflateRect(&fillRect, -1, -1);
    if (fillRect.right > fillRect.left) {
        HBRUSH fill = CreateSolidBrush(RGB(215, 188, 72));
        FillRect(hdc, &fillRect, fill);
        DeleteObject(fill);
    }

    std::wostringstream summary;
    summary << L"Startup " << current << L"/" << total;
    if (state && state->shaderTotal > 0) {
        summary << L"    Shaders " << state->shaderDone << L"/" << state->shaderTotal
                << L" (compiled " << state->shaderCompiled << L", cached " << state->shaderCached << L")";
    }
    RECT summaryRect{left, barTop + 22, left + contentW, barTop + 48};
    SetTextColor(hdc, RGB(132, 128, 102));
    DrawTextW(hdc, summary.str().c_str(), -1, &summaryRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);
    DeleteObject(detailFont);
}

void PaintLoadingOverlayBuffered(HWND hwnd, HDC targetDc) {
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int width = std::max(1, static_cast<int>(rc.right - rc.left));
    int height = std::max(1, static_cast<int>(rc.bottom - rc.top));
    HDC memDc = CreateCompatibleDC(targetDc);
    HBITMAP memBmp = CreateCompatibleBitmap(targetDc, width, height);
    if (!memDc || !memBmp) {
        if (memBmp) DeleteObject(memBmp);
        if (memDc) DeleteDC(memDc);
        DrawLoadingOverlay(hwnd, targetDc);
        return;
    }
    HGDIOBJ oldBmp = SelectObject(memDc, memBmp);
    DrawLoadingOverlay(hwnd, memDc);
    BitBlt(targetDc, 0, 0, width, height, memDc, 0, 0, SRCCOPY);
    SelectObject(memDc, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDc);
}
