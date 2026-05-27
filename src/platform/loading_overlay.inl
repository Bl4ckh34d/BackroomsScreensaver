// Loading overlay window and startup-progress bridge.
// Included from main.cpp before the main window procedure.

constexpr int kLoadingOverlayTimerId = 6101;
const wchar_t* kLoadingOverlayClass = L"BackroomsMazeLoadingOverlay";

struct LoadingOverlayState {
    std::wstring phase = L"Starting renderer";
    std::wstring detail = L"";
    bool brandedSplash = false;
    int current = 0;
    int total = 1;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
    int frame = 0;
};

LoadingOverlayState* LoadingState(HWND hwnd) {
    return reinterpret_cast<LoadingOverlayState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

void DrawLoadingSpinner(HDC hdc, int cx, int cy, int radius, int frame) {
    constexpr int dots = 12;
    for (int i = 0; i < dots; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(dots)) * kPi * 2.0f;
        int age = (i - frame) % dots;
        if (age < 0) age += dots;
        int intensity = 72 + (dots - age) * 13;
        intensity = std::clamp(intensity, 72, 228);
        int x = cx + static_cast<int>(std::cos(angle) * radius);
        int y = cy + static_cast<int>(std::sin(angle) * radius);
        int dotRadius = std::max(2, radius / 6);
        HBRUSH brush = CreateSolidBrush(RGB(intensity, intensity - 8, intensity / 2));
        HGDIOBJ oldBrush = SelectObject(hdc, brush);
        HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        Ellipse(hdc, x - dotRadius, y - dotRadius, x + dotRadius + 1, y + dotRadius + 1);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
        DeleteObject(brush);
    }
}

const ImageRGBA& LoadingOverlayLogo() {
    static ImageRGBA logo;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        const std::array<std::filesystem::path, 3> candidates = {
            ModuleDirectory() / L"assets" / L"branding" / L"NeuralForge_Solutions.png",
            std::filesystem::current_path() / L"assets" / L"branding" / L"NeuralForge_Solutions.png",
            std::filesystem::current_path().parent_path() / L"assets" / L"branding" / L"NeuralForge_Solutions.png"
        };
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec) && LoadImageWic(candidate, 0, 0, logo)) break;
        }
    }
    return logo;
}

void DrawTintedLoadingLogo(HDC hdc, const RECT& rc, const LoadingOverlayState* state) {
    const ImageRGBA& logo = LoadingOverlayLogo();
    int width = std::max<LONG>(1, rc.right - rc.left);
    int height = std::max<LONG>(1, rc.bottom - rc.top);
    int logoSize = std::clamp(std::min(width, height) / 4, 132, 248);
    int x = rc.left + (width - logoSize) / 2;
    int y = rc.top + height / 2 - logoSize / 2 - std::clamp(height / 16, 24, 58);

    if (logo.Valid()) {
        static std::vector<uint8_t> tinted;
        static int tintedW = 0;
        static int tintedH = 0;
        if (tintedW != logo.width || tintedH != logo.height || tinted.empty()) {
            tintedW = logo.width;
            tintedH = logo.height;
            tinted.assign(static_cast<size_t>(tintedW) * static_cast<size_t>(tintedH) * 4, 0);
            bool hasTransparency = false;
            for (int py = 0; py < tintedH && !hasTransparency; ++py) {
                for (int px = 0; px < tintedW; ++px) {
                    size_t src = static_cast<size_t>((py * tintedW + px) * 4);
                    if (logo.pixels[src + 3] < 250) {
                        hasTransparency = true;
                        break;
                    }
                }
            }
            for (int py = 0; py < tintedH; ++py) {
                for (int px = 0; px < tintedW; ++px) {
                    size_t src = static_cast<size_t>((py * tintedW + px) * 4);
                    uint8_t b = logo.pixels[src + 0];
                    uint8_t g = logo.pixels[src + 1];
                    uint8_t r = logo.pixels[src + 2];
                    uint8_t srcA = logo.pixels[src + 3];
                    uint8_t rgbCoverage = std::max(r, std::max(g, b));
                    uint8_t darkCoverage = static_cast<uint8_t>(255 - std::min(r, std::min(g, b)));
                    uint8_t a = hasTransparency ? srcA : std::max(rgbCoverage, darkCoverage);
                    size_t dst = src;
                    tinted[dst + 0] = a;
                    tinted[dst + 1] = a;
                    tinted[dst + 2] = a;
                    tinted[dst + 3] = 255;
                }
            }
        }

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = tintedW;
        bmi.bmiHeader.biHeight = -tintedH;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        SetStretchBltMode(hdc, HALFTONE);
        StretchDIBits(hdc, x, y, logoSize, logoSize, 0, 0, tintedW, tintedH,
            tinted.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
    }

    SetBkMode(hdc, TRANSPARENT);
    HFONT logoFont = CreateFontW(-28, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT detailFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, logoFont);
    SetTextColor(hdc, RGB(245, 245, 245));
    RECT nameRect{rc.left + 24, y + logoSize + 16, rc.right - 24, y + logoSize + 54};
    DrawTextW(hdc, L"NeuralForge Solutions", -1, &nameRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    SelectObject(hdc, detailFont);
    SetTextColor(hdc, RGB(126, 119, 91));
    std::wstring detail = state && !state->detail.empty() ? state->detail : L"Preparing renderer";
    RECT detailRect{rc.left + 24, nameRect.bottom + 4, rc.right - 24, nameRect.bottom + 30};
    DrawTextW(hdc, detail.c_str(), -1, &detailRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    SelectObject(hdc, oldFont);
    DeleteObject(logoFont);
    DeleteObject(detailFont);
}

void DrawLoadingOverlay(HWND hwnd, HDC hdc) {
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int width = std::max(1, static_cast<int>(rc.right - rc.left));
    int height = std::max(1, static_cast<int>(rc.bottom - rc.top));
    LoadingOverlayState* state = LoadingState(hwnd);

    HBRUSH bg = CreateSolidBrush(RGB(8, 8, 6));
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

LRESULT CALLBACK LoadingOverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCCREATE: {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return TRUE;
    }
    case WM_CREATE:
        SetTimer(hwnd, kLoadingOverlayTimerId, 120, nullptr);
        return 0;
    case WM_TIMER:
        if (wParam == kLoadingOverlayTimerId) {
            if (LoadingOverlayState* state = LoadingState(hwnd)) {
                state->frame = (state->frame + 1) % 12;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawLoadingOverlay(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_NCDESTROY:
        KillTimer(hwnd, kLoadingOverlayTimerId);
        delete LoadingState(hwnd);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void RegisterLoadingOverlayClass(HINSTANCE hInstance) {
    static bool registered = false;
    if (registered) return;
    WNDCLASSW wc{};
    wc.lpfnWndProc = LoadingOverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kLoadingOverlayClass;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);
    registered = true;
}

void ResizeLoadingOverlay(HWND parent, HWND overlay) {
    if (!parent || !overlay) return;
    RECT rc{};
    GetClientRect(parent, &rc);
    MoveWindow(overlay, 0, 0,
        std::max(1, static_cast<int>(rc.right - rc.left)),
        std::max(1, static_cast<int>(rc.bottom - rc.top)), TRUE);
}

HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash) {
    RegisterLoadingOverlayClass(hInstance);
    auto* state = new LoadingOverlayState();
    state->brandedSplash = brandedSplash;
    RECT rc{};
    GetClientRect(parent, &rc);
    HWND overlay = CreateWindowExW(0, kLoadingOverlayClass, nullptr, WS_CHILD | WS_VISIBLE,
        0, 0, std::max(1, static_cast<int>(rc.right - rc.left)),
        std::max(1, static_cast<int>(rc.bottom - rc.top)),
        parent, nullptr, hInstance, state);
    if (!overlay) {
        delete state;
        return nullptr;
    }
    SetWindowPos(overlay, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    UpdateWindow(overlay);
    return overlay;
}

void SetLoadingOverlayProgress(HWND overlay, const StartupProgressUpdate& update) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (!state) return;
    state->phase = update.phase ? update.phase : L"Loading";
    state->detail = update.detail ? update.detail : L"";
    state->current = std::clamp(update.current, 0, std::max(1, update.total));
    state->total = std::max(1, update.total);
    state->shaderDone = std::clamp(update.shaderDone, 0, std::max(0, update.shaderTotal));
    state->shaderTotal = std::max(0, update.shaderTotal);
    state->shaderCompiled = std::max(0, update.shaderCompiled);
    state->shaderCached = std::max(0, update.shaderCached);
    state->frame = (state->frame + 1) % 12;
    RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (!state) return;
    int total = std::max(state->total + 1, state->current + 1);
    state->phase = phase ? phase : L"Loading";
    state->detail = detail ? detail : L"";
    state->total = std::max(1, total);
    state->current = complete ? state->total : std::max(0, state->total - 1);
    state->frame = (state->frame + 1) % 12;
    RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

void LoadingProgressCallback(void* context, const StartupProgressUpdate& update) {
    SetLoadingOverlayProgress(static_cast<HWND>(context), update);
}
