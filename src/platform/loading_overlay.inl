// Loading overlay window and startup-progress bridge.
// Included from main.cpp before the main window procedure.

constexpr int kLoadingOverlayTimerId = 6101;
constexpr UINT kLoadingOverlayCloseMessage = WM_APP + 6102;
constexpr UINT kLoadingOverlayProgressMessage = WM_APP + 6103;
constexpr UINT kLoadingOverlayStatusMessage = WM_APP + 6104;
const wchar_t* kLoadingOverlayClass = L"BackroomsMazeLoadingOverlay";
constexpr ULONGLONG kBrandedIntroReadyMs = 7000;

struct LoadingOverlayThreadInfo {
    HANDLE handle = nullptr;
    DWORD threadId = 0;
    bool brandedSplash = false;
    ULONGLONG createdTick = 0;
};
std::unordered_map<HWND, LoadingOverlayThreadInfo> gLoadingOverlayThreads;

struct LoadingOverlayState {
    CRITICAL_SECTION lock{};
    std::wstring phase = L"Starting renderer";
    std::wstring detail = L"";
    bool brandedSplash = false;
    bool threadedPopup = false;
    int current = 0;
    int total = 1;
    int fineCurrent = 0;
    int fineTotal = 0;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
    int frame = 0;
    ULONGLONG createdTick = 0;
    ULONGLONG lastUpdateTick = 0;
    ULONGLONG completeTick = 0;
    bool complete = false;
};

struct LoadingOverlayThreadStart {
    HWND owner = nullptr;
    HINSTANCE hInstance = nullptr;
    HANDLE ready = nullptr;
    HWND overlay = nullptr;
    DWORD threadId = 0;
    ULONGLONG createdTick = 0;
    bool brandedSplash = false;
};

struct LoadingOverlayPostedUpdate {
    std::wstring phase;
    std::wstring detail;
    bool complete = false;
    int current = 0;
    int total = 1;
    int fineCurrent = 0;
    int fineTotal = 0;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
};

struct LoadingOverlaySnapshot {
    std::wstring phase;
    std::wstring detail;
    bool brandedSplash = false;
    int current = 0;
    int total = 1;
    int fineCurrent = 0;
    int fineTotal = 0;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
    int frame = 0;
    ULONGLONG createdTick = 0;
    ULONGLONG lastUpdateTick = 0;
    ULONGLONG completeTick = 0;
    bool complete = false;
};

constexpr uint8_t kLoadingBgR = 0;
constexpr uint8_t kLoadingBgG = 0;
constexpr uint8_t kLoadingBgB = 0;
constexpr uint8_t kLoadingTitleR = 246;
constexpr uint8_t kLoadingTitleG = 235;
constexpr uint8_t kLoadingTitleB = 190;

uint8_t LoadingLerpChannel(uint8_t a, uint8_t b, float t) {
    return static_cast<uint8_t>(std::clamp(Lerp(static_cast<float>(a), static_cast<float>(b), Clamp01(t)), 0.0f, 255.0f));
}

COLORREF LoadingFadeColor(uint8_t r, uint8_t g, uint8_t b, float alpha) {
    return RGB(
        LoadingLerpChannel(kLoadingBgR, r, alpha),
        LoadingLerpChannel(kLoadingBgG, g, alpha),
        LoadingLerpChannel(kLoadingBgB, b, alpha));
}

LoadingOverlayState* LoadingState(HWND hwnd) {
    return reinterpret_cast<LoadingOverlayState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

void LockLoadingState(LoadingOverlayState* state) {
    if (state) EnterCriticalSection(&state->lock);
}

void UnlockLoadingState(LoadingOverlayState* state) {
    if (state) LeaveCriticalSection(&state->lock);
}

LoadingOverlaySnapshot CaptureLoadingOverlaySnapshot(HWND hwnd) {
    LoadingOverlaySnapshot snap{};
    if (LoadingOverlayState* state = LoadingState(hwnd)) {
        LockLoadingState(state);
        snap.phase = state->phase;
        snap.detail = state->detail;
        snap.brandedSplash = state->brandedSplash;
        snap.current = state->current;
        snap.total = state->total;
        snap.fineCurrent = state->fineCurrent;
        snap.fineTotal = state->fineTotal;
        snap.shaderDone = state->shaderDone;
        snap.shaderTotal = state->shaderTotal;
        snap.shaderCompiled = state->shaderCompiled;
        snap.shaderCached = state->shaderCached;
        snap.frame = state->frame;
        snap.createdTick = state->createdTick;
        snap.lastUpdateTick = state->lastUpdateTick;
        snap.completeTick = state->completeTick;
        snap.complete = state->complete;
        UnlockLoadingState(state);
    }
    return snap;
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
    if (!loaded || !logo.Valid()) {
        ScopedCom com;
        if (!com.Ok()) return logo;
        const std::array<std::filesystem::path, 3> candidates = {
            ModuleDirectory() / L"assets" / L"branding" / L"NeuralForge_Solutions.png",
            std::filesystem::current_path() / L"assets" / L"branding" / L"NeuralForge_Solutions.png",
            std::filesystem::current_path().parent_path() / L"assets" / L"branding" / L"NeuralForge_Solutions.png"
        };
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec) && LoadImageWic(candidate, 0, 0, logo)) break;
        }
        loaded = logo.Valid();
    }
    return logo;
}

void DrawTintedLoadingLogo(HDC hdc, const RECT& rc, const LoadingOverlaySnapshot* state) {
    const ImageRGBA& logo = LoadingOverlayLogo();
    int width = std::max<LONG>(1, rc.right - rc.left);
    int height = std::max<LONG>(1, rc.bottom - rc.top);
    int logoSize = std::clamp(static_cast<int>(std::min(width, height) * 0.46f), 240, 430);
    int x = rc.left + (width - logoSize) / 2;
    int y = rc.top + height / 2 - logoSize / 2 - std::clamp(height / 24, 18, 42);
    float age = 0.0f;
    float doneAge = -1.0f;
    if (state && state->createdTick != 0) {
        age = static_cast<float>(GetTickCount64() - state->createdTick) * 0.001f;
        if (state->complete && state->completeTick != 0) {
            doneAge = static_cast<float>(GetTickCount64() - state->completeTick) * 0.001f;
        }
    }
    float logoAlpha = SmoothStep(0.45f, 1.85f, age) * (1.0f - SmoothStep(3.40f, 4.45f, age));
    float titleAlpha = SmoothStep(4.85f, 6.65f, age);
    if (doneAge >= 0.0f) {
        titleAlpha *= 1.0f - SmoothStep(0.10f, 0.95f, doneAge);
    }

    if (logo.Valid() && logoAlpha > 0.002f) {
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

    SetBkMode(hdc, TRANSPARENT);
    int titleTextWidth = std::clamp(width * 34 / 100, 220, 520);
    int titleSize = std::clamp(height / 9, 62, 138);
    if (titleAlpha > 0.002f) {
        HFONT titleFont = CreateFontW(-titleSize, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Arial");
        HGDIOBJ oldFont = SelectObject(hdc, titleFont);
        SetTextColor(hdc, LoadingFadeColor(kLoadingTitleR, kLoadingTitleG, kLoadingTitleB, titleAlpha));
        RECT titleRect{rc.left + 24, rc.top + height / 2 - titleSize, rc.right - 24, rc.top + height / 2 + titleSize};
        SIZE textExtent{};
        if (GetTextExtentPoint32W(hdc, L"BACKROOMS", 9, &textExtent) && textExtent.cx > 0) {
            titleTextWidth = std::clamp(static_cast<int>(textExtent.cx), 180, std::max(180, width - 48));
        }
        DrawTextW(hdc, L"BACKROOMS", -1, &titleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
        SelectObject(hdc, oldFont);
        DeleteObject(titleFont);
    }
    if (titleAlpha > 0.18f && state) {
        float detailAlpha = Clamp01((titleAlpha - 0.18f) / 0.82f) * (doneAge >= 0.0f ? (1.0f - SmoothStep(0.05f, 0.72f, doneAge)) : 1.0f);
        int barWidth = std::min(titleTextWidth, width - 48);
        int barHeight = std::clamp(height / 58, 12, 22);
        int barLeft = rc.left + (width - barWidth) / 2;
        int barTop = rc.top + height / 2 + std::clamp(titleSize * 2 / 3, 46, 86);
        RECT barRect{barLeft, barTop, barLeft + barWidth, barTop + barHeight};
        HBRUSH barBg = CreateSolidBrush(LoadingFadeColor(32, 30, 22, detailAlpha * 0.82f));
        FillRect(hdc, &barRect, barBg);
        DeleteObject(barBg);

        int total = state->fineTotal > 0 ? std::max(1, state->fineTotal) : std::max(1, state->total);
        int current = state->fineTotal > 0 ? std::clamp(state->fineCurrent, 0, total) : state->current;
        float progress = state->complete ? 1.0f : Clamp01(static_cast<float>(current) / static_cast<float>(total));
        if (!state->complete && current > 0 && current < total) {
            ULONGLONG updateTick = state->lastUpdateTick != 0 ? state->lastUpdateTick : state->createdTick;
            float sinceUpdate = updateTick != 0
                ? static_cast<float>(GetTickCount64() - updateTick) * 0.001f
                : 0.0f;
            float stepSize = 1.0f / static_cast<float>(total);
            float intraStep = SmoothStep(0.0f, 2.25f, sinceUpdate) * stepSize * 0.72f;
            if (state->shaderTotal > 0 && state->shaderDone < state->shaderTotal) {
                intraStep = std::max(intraStep, stepSize * 0.58f *
                    SmoothStep(0.0f, 1.35f, sinceUpdate));
            }
            progress = std::min(progress + intraStep,
                static_cast<float>(current + 1) / static_cast<float>(total) - stepSize * 0.12f);
        }
        RECT fillRect = barRect;
        fillRect.right = fillRect.left + static_cast<int>(std::round(static_cast<float>(barWidth) * progress));
        if (fillRect.right > fillRect.left) {
            HBRUSH barFill = CreateSolidBrush(LoadingFadeColor(kLoadingTitleR, kLoadingTitleG, kLoadingTitleB, detailAlpha));
            FillRect(hdc, &fillRect, barFill);
            DeleteObject(barFill);
        }
    }
    if (state && state->brandedSplash) return;

    HFONT detailFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, detailFont);
    SetTextColor(hdc, RGB(126, 119, 91));
    std::wstring detail = state && !state->detail.empty() ? state->detail : L"Preparing renderer";
    RECT detailRect{rc.left + 24, y + logoSize + 18, rc.right - 24, y + logoSize + 44};
    DrawTextW(hdc, detail.c_str(), -1, &detailRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    SelectObject(hdc, oldFont);
    DeleteObject(detailFont);
}

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

LRESULT CALLBACK LoadingOverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCCREATE: {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return TRUE;
    }
    case WM_CREATE:
        SetTimer(hwnd, kLoadingOverlayTimerId, 16, nullptr);
        return 0;
    case WM_TIMER:
        if (wParam == kLoadingOverlayTimerId) {
            if (LoadingOverlayState* state = LoadingState(hwnd)) {
                LockLoadingState(state);
                state->frame = (state->frame + 1) % 12;
                UnlockLoadingState(state);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_SETCURSOR:
        SetCursor(nullptr);
        return TRUE;
    default:
        if (msg == kLoadingOverlayCloseMessage) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (msg == kLoadingOverlayProgressMessage || msg == kLoadingOverlayStatusMessage) {
            std::unique_ptr<LoadingOverlayPostedUpdate> update(reinterpret_cast<LoadingOverlayPostedUpdate*>(lParam));
            if (update) {
                if (LoadingOverlayState* state = LoadingState(hwnd)) {
                    LockLoadingState(state);
                    state->phase = update->phase.empty() ? L"Loading" : update->phase;
                    state->detail = update->detail;
                    if (msg == kLoadingOverlayProgressMessage) {
                        state->current = std::clamp(update->current, 0, std::max(1, update->total));
                        state->total = std::max(1, update->total);
                        state->fineCurrent = std::clamp(update->fineCurrent, 0, std::max(1, update->fineTotal));
                        state->fineTotal = std::max(0, update->fineTotal);
                        state->shaderDone = std::clamp(update->shaderDone, 0, std::max(0, update->shaderTotal));
                        state->shaderTotal = std::max(0, update->shaderTotal);
                        state->shaderCompiled = std::max(0, update->shaderCompiled);
                        state->shaderCached = std::max(0, update->shaderCached);
                    } else {
                        int total = std::max(state->total + 1, state->current + 1);
                        if (update->complete && !state->complete) {
                            state->complete = true;
                            state->completeTick = GetTickCount64();
                        }
                        state->total = std::max(1, total);
                        state->current = update->complete ? state->total : std::max(0, state->total - 1);
                        state->fineTotal = std::max(state->fineTotal, state->total);
                        state->fineCurrent = update->complete ? state->fineTotal : std::min(state->fineCurrent, state->fineTotal);
                    }
                    state->lastUpdateTick = GetTickCount64();
                    state->frame = (state->frame + 1) % 12;
                    UnlockLoadingState(state);
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);
        PaintLoadingOverlayBuffered(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_NCDESTROY:
        KillTimer(hwnd, kLoadingOverlayTimerId);
        if (LoadingOverlayState* state = LoadingState(hwnd)) {
            bool shouldQuit = state->threadedPopup;
            DeleteCriticalSection(&state->lock);
            delete state;
            if (shouldQuit) PostQuitMessage(0);
        }
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
    wc.hCursor = nullptr;
    RegisterClassW(&wc);
    registered = true;
}

void ResizeLoadingOverlay(HWND parent, HWND overlay) {
    if (!parent || !overlay) return;
    RECT rc{};
    GetClientRect(parent, &rc);
    LoadingOverlayState* state = LoadingState(overlay);
    bool popup = false;
    if (state) {
        LockLoadingState(state);
        popup = state->threadedPopup;
        UnlockLoadingState(state);
    }
    if (popup) {
        POINT tl{rc.left, rc.top};
        ClientToScreen(parent, &tl);
        SetWindowPos(overlay, HWND_TOP, tl.x, tl.y,
            std::max(1, static_cast<int>(rc.right - rc.left)),
            std::max(1, static_cast<int>(rc.bottom - rc.top)),
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
    } else {
        MoveWindow(overlay, 0, 0,
            std::max(1, static_cast<int>(rc.right - rc.left)),
            std::max(1, static_cast<int>(rc.bottom - rc.top)), TRUE);
    }
}

void PumpLoadingOverlayMessages() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

DWORD WINAPI LoadingOverlayThreadProc(LPVOID param) {
    auto* start = static_cast<LoadingOverlayThreadStart*>(param);
    start->threadId = GetCurrentThreadId();
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    RegisterLoadingOverlayClass(start->hInstance);
    if (start->brandedSplash) {
        (void)LoadingOverlayLogo();
    }
    auto* state = new LoadingOverlayState();
    InitializeCriticalSection(&state->lock);
    state->brandedSplash = start->brandedSplash;
    state->threadedPopup = true;
    state->createdTick = GetTickCount64();
    state->lastUpdateTick = state->createdTick;
    RECT rc{};
    GetClientRect(start->owner, &rc);
    POINT tl{rc.left, rc.top};
    ClientToScreen(start->owner, &tl);
    int width = std::max(1, static_cast<int>(rc.right - rc.left));
    int height = std::max(1, static_cast<int>(rc.bottom - rc.top));
    HWND overlay = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kLoadingOverlayClass,
        nullptr,
        WS_POPUP | WS_VISIBLE,
        tl.x,
        tl.y,
        width,
        height,
        start->owner,
        nullptr,
        start->hInstance,
        state);
    if (!overlay) {
        DeleteCriticalSection(&state->lock);
        delete state;
        start->overlay = nullptr;
        SetEvent(start->ready);
        return 0;
    }
    start->overlay = overlay;
    SetWindowPos(overlay, HWND_TOP, tl.x, tl.y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    UpdateWindow(overlay);
    SetEvent(start->ready);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

HWND CreateThreadedLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash) {
    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!ready) return nullptr;

    LoadingOverlayThreadStart start{};
    start.owner = parent;
    start.hInstance = hInstance;
    start.ready = ready;
    start.createdTick = GetTickCount64();
    start.brandedSplash = brandedSplash;

    DWORD threadId = 0;
    HANDLE thread = CreateThread(nullptr, 0, LoadingOverlayThreadProc, &start, 0, &threadId);
    if (!thread) {
        CloseHandle(ready);
        return nullptr;
    }
    SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);

    DWORD waitResult = WaitForSingleObject(ready, 1500);
    CloseHandle(ready);
    if (waitResult != WAIT_OBJECT_0 || !start.overlay) {
        PostThreadMessageW(threadId, WM_QUIT, 0, 0);
        WaitForSingleObject(thread, 400);
        CloseHandle(thread);
        return nullptr;
    }

    LoadingOverlayThreadInfo info{};
    info.handle = thread;
    info.threadId = start.threadId != 0 ? start.threadId : threadId;
    info.brandedSplash = brandedSplash;
    info.createdTick = start.createdTick;
    if (LoadingOverlayState* state = LoadingState(start.overlay)) {
        LockLoadingState(state);
        info.createdTick = state->createdTick;
        UnlockLoadingState(state);
    }
    gLoadingOverlayThreads[start.overlay] = info;
    return start.overlay;
}

bool LoadingOverlayHasIndependentSplash(HWND overlay) {
    auto it = gLoadingOverlayThreads.find(overlay);
    return it != gLoadingOverlayThreads.end() && it->second.brandedSplash && it->second.handle;
}

void WaitForLoadingOverlayIntro(HWND overlay) {
    if (!overlay) return;
    ULONGLONG createdTick = 0;
    auto it = gLoadingOverlayThreads.find(overlay);
    if (it != gLoadingOverlayThreads.end() && it->second.brandedSplash) {
        createdTick = it->second.createdTick;
    } else if (LoadingOverlayState* state = LoadingState(overlay)) {
        LockLoadingState(state);
        if (state->brandedSplash) createdTick = state->createdTick;
        UnlockLoadingState(state);
    }
    if (createdTick == 0) return;

    ULONGLONG readyAt = createdTick + kBrandedIntroReadyMs;
    while (GetTickCount64() < readyAt && IsWindow(overlay)) {
        PumpLoadingOverlayMessages();
        Sleep(16);
    }
}

HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash) {
    if (brandedSplash) {
        if (HWND threaded = CreateThreadedLoadingOverlay(parent, hInstance, brandedSplash)) {
            return threaded;
        }
    }

    RegisterLoadingOverlayClass(hInstance);
    if (brandedSplash) {
        (void)LoadingOverlayLogo();
    }
    auto* state = new LoadingOverlayState();
    InitializeCriticalSection(&state->lock);
    state->brandedSplash = brandedSplash;
    state->threadedPopup = false;
    state->createdTick = GetTickCount64();
    state->lastUpdateTick = state->createdTick;
    RECT rc{};
    GetClientRect(parent, &rc);
    HWND overlay = CreateWindowExW(0, kLoadingOverlayClass, nullptr, WS_CHILD | WS_VISIBLE,
        0, 0, std::max(1, static_cast<int>(rc.right - rc.left)),
        std::max(1, static_cast<int>(rc.bottom - rc.top)),
        parent, nullptr, hInstance, state);
    if (!overlay) {
        DeleteCriticalSection(&state->lock);
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
    if (state && !state->threadedPopup) {
        LockLoadingState(state);
        state->phase = update.phase ? update.phase : L"Loading";
        state->detail = update.detail ? update.detail : L"";
        state->current = std::clamp(update.current, 0, std::max(1, update.total));
        state->total = std::max(1, update.total);
        state->fineCurrent = std::clamp(update.fineCurrent, 0, std::max(1, update.fineTotal));
        state->fineTotal = std::max(0, update.fineTotal);
        state->shaderDone = std::clamp(update.shaderDone, 0, std::max(0, update.shaderTotal));
        state->shaderTotal = std::max(0, update.shaderTotal);
        state->shaderCompiled = std::max(0, update.shaderCompiled);
        state->shaderCached = std::max(0, update.shaderCached);
        state->lastUpdateTick = GetTickCount64();
        state->frame = (state->frame + 1) % 12;
        UnlockLoadingState(state);
        RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
        PumpLoadingOverlayMessages();
    } else {
        auto* posted = new LoadingOverlayPostedUpdate();
        posted->phase = update.phase ? update.phase : L"Loading";
        posted->detail = update.detail ? update.detail : L"";
        posted->current = update.current;
        posted->total = update.total;
        posted->fineCurrent = update.fineCurrent;
        posted->fineTotal = update.fineTotal;
        posted->shaderDone = update.shaderDone;
        posted->shaderTotal = update.shaderTotal;
        posted->shaderCompiled = update.shaderCompiled;
        posted->shaderCached = update.shaderCached;
        if (!PostMessageW(overlay, kLoadingOverlayProgressMessage, 0, reinterpret_cast<LPARAM>(posted))) {
            delete posted;
        }
    }
}

void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (state && !state->threadedPopup) {
        LockLoadingState(state);
        int total = std::max(state->total + 1, state->current + 1);
        state->phase = phase ? phase : L"Loading";
        state->detail = detail ? detail : L"";
        if (complete && !state->complete) {
            state->complete = true;
            state->completeTick = GetTickCount64();
        }
        state->total = std::max(1, total);
        state->current = complete ? state->total : std::max(0, state->total - 1);
        state->fineTotal = std::max(state->fineTotal, state->total);
        state->fineCurrent = complete ? state->fineTotal : std::min(state->fineCurrent, state->fineTotal);
        state->lastUpdateTick = GetTickCount64();
        state->frame = (state->frame + 1) % 12;
        UnlockLoadingState(state);
        RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
        PumpLoadingOverlayMessages();
    } else {
        auto* posted = new LoadingOverlayPostedUpdate();
        posted->phase = phase ? phase : L"Loading";
        posted->detail = detail ? detail : L"";
        posted->complete = complete;
        if (!PostMessageW(overlay, kLoadingOverlayStatusMessage, 0, reinterpret_cast<LPARAM>(posted))) {
            delete posted;
        }
    }
}

void CloseLoadingOverlayWindow(HWND overlay) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (state && !state->threadedPopup) {
        DestroyWindow(overlay);
        PumpLoadingOverlayMessages();
        return;
    }
    LoadingOverlayThreadInfo threadInfo{};
    auto it = gLoadingOverlayThreads.find(overlay);
    if (it != gLoadingOverlayThreads.end()) {
        threadInfo = it->second;
        gLoadingOverlayThreads.erase(it);
    }
    ShowWindowAsync(overlay, SW_HIDE);
    SetWindowPos(overlay, nullptr, 0, 0, 0, 0,
        SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    DWORD_PTR closeResult = 0;
    SendMessageTimeoutW(overlay, kLoadingOverlayCloseMessage, 0, 0,
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 700, &closeResult);
    ULONGLONG start = GetTickCount64();
    if (threadInfo.handle) {
        bool quitPosted = false;
        while (WaitForSingleObject(threadInfo.handle, 10) == WAIT_TIMEOUT && GetTickCount64() - start < 1200) {
            if (!quitPosted && GetTickCount64() - start > 140 && threadInfo.threadId != 0) {
                PostThreadMessageW(threadInfo.threadId, WM_QUIT, 0, 0);
                quitPosted = true;
            }
            PumpLoadingOverlayMessages();
        }
        CloseHandle(threadInfo.handle);
    } else {
        while (IsWindow(overlay) && GetTickCount64() - start < 1200) {
            PumpLoadingOverlayMessages();
            Sleep(10);
        }
    }
    PumpLoadingOverlayMessages();
}

void FinishLoadingOverlay(HWND overlay) {
    if (!overlay) return;
    bool branded = false;
    ULONGLONG createdTick = GetTickCount64();
    auto it = gLoadingOverlayThreads.find(overlay);
    if (it != gLoadingOverlayThreads.end()) {
        branded = it->second.brandedSplash;
        createdTick = it->second.createdTick;
    } else if (LoadingOverlayState* state = LoadingState(overlay)) {
        LockLoadingState(state);
        branded = state->brandedSplash;
        createdTick = state->createdTick;
        UnlockLoadingState(state);
    }
    if (!branded) {
        CloseLoadingOverlayWindow(overlay);
        return;
    }

    ULONGLONG completeTick = GetTickCount64();
    ULONGLONG minVisibleUntil = createdTick + kBrandedIntroReadyMs + 600;
    ULONGLONG fadeUntil = completeTick + 1050;
    ULONGLONG until = std::max(minVisibleUntil, fadeUntil);
    while (GetTickCount64() < until && IsWindow(overlay)) {
        InvalidateRect(overlay, nullptr, FALSE);
        PumpLoadingOverlayMessages();
        Sleep(16);
    }
    CloseLoadingOverlayWindow(overlay);
}

void LoadingProgressCallback(void* context, const StartupProgressUpdate& update) {
    SetLoadingOverlayProgress(static_cast<HWND>(context), update);
}
