#pragma once

#include "../debug/startup_progress.h"

HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash = false);
void ResizeLoadingOverlay(HWND parent, HWND overlay);
void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete);
void CloseLoadingOverlayWindow(HWND overlay);
void FinishLoadingOverlay(HWND overlay);
bool LoadingOverlayHasIndependentSplash(HWND overlay);
void WaitForLoadingOverlayIntro(HWND overlay);
void LoadingProgressCallback(void* context, const StartupProgressUpdate& update);
