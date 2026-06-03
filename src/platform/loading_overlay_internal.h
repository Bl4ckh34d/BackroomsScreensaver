#pragma once

#include "platform_headers.h"
#include "../core/math_utils.h"
#include "../core/constants.h"
#include "../config/settings.h"

#include "loading_overlay.h"

constexpr int kLoadingOverlayTimerId = 6101;
constexpr UINT kLoadingOverlayCloseMessage = WM_APP + 6102;
constexpr UINT kLoadingOverlayProgressMessage = WM_APP + 6103;
constexpr UINT kLoadingOverlayStatusMessage = WM_APP + 6104;
constexpr const wchar_t* kLoadingOverlayClass = L"BackroomsMazeLoadingOverlay";
constexpr ULONGLONG kBrandedIntroReadyMs = 7000;

constexpr uint8_t kLoadingBgR = 0;
constexpr uint8_t kLoadingBgG = 0;
constexpr uint8_t kLoadingBgB = 0;
constexpr uint8_t kLoadingTitleR = 246;
constexpr uint8_t kLoadingTitleG = 235;
constexpr uint8_t kLoadingTitleB = 190;

struct LoadingOverlayThreadInfo {
    HANDLE handle = nullptr;
    DWORD threadId = 0;
    bool brandedSplash = false;
    ULONGLONG createdTick = 0;
};

extern std::unordered_map<HWND, LoadingOverlayThreadInfo> gLoadingOverlayThreads;

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

struct LoadingBrandTitleLayout {
    int textWidth = 0;
    int bottom = 0;
};

uint8_t LoadingLerpChannel(uint8_t a, uint8_t b, float t);
COLORREF LoadingFadeColor(uint8_t r, uint8_t g, uint8_t b, float alpha);
LoadingOverlayState* LoadingState(HWND hwnd);
void LockLoadingState(LoadingOverlayState* state);
void UnlockLoadingState(LoadingOverlayState* state);
LoadingOverlaySnapshot CaptureLoadingOverlaySnapshot(HWND hwnd);
void DrawLoadingSpinner(HDC hdc, int cx, int cy, int radius, int frame);
const ImageRGBA& LoadingOverlayLogo();
const ImageRGBA& LoadingOverlayTitleLogo();
void DrawLoadingBrandMark(HDC hdc, int x, int y, int logoSize, float logoAlpha);
LoadingBrandTitleLayout DrawLoadingBrandTitle(HDC hdc, const RECT& rc, int width, int height, float titleAlpha);
void DrawLoadingBrandProgress(
    HDC hdc,
    const RECT& rc,
    const LoadingOverlaySnapshot& state,
    int width,
    int height,
    float titleAlpha,
    float doneAge,
    const LoadingBrandTitleLayout& title);
void DrawLoadingBrandFallbackDetail(
    HDC hdc,
    const RECT& rc,
    const LoadingOverlaySnapshot* state,
    int x,
    int y,
    int logoSize);
void DrawTintedLoadingLogo(HDC hdc, const RECT& rc, const LoadingOverlaySnapshot* state);
void DrawLoadingOverlay(HWND hwnd, HDC hdc);
void PaintLoadingOverlayBuffered(HWND hwnd, HDC targetDc);
LRESULT CALLBACK LoadingOverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RegisterLoadingOverlayClass(HINSTANCE hInstance);
void PumpLoadingOverlayMessages();
DWORD WINAPI LoadingOverlayThreadProc(LPVOID param);
HWND CreateThreadedLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash);
