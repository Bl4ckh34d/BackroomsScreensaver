#include "app_state.inl"

enum class ConfigDialogMode {
    Full,
    Game,
    Debug
};
void ShowConfig(HWND owner, ConfigDialogMode mode = ConfigDialogMode::Full);
HWND CreateEmbeddedConfig(HWND parent, ConfigDialogMode mode);
#if defined(BACKROOMS_GAME_EXE)
HWND CreateGameSettingsPanel(HWND parent);
#endif
HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash = false);
void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete);
void CloseLoadingOverlayWindow(HWND overlay);
void FinishLoadingOverlay(HWND overlay);
bool LoadingOverlayHasIndependentSplash(HWND overlay);
void WaitForLoadingOverlayIntro(HWND overlay);
void LoadingProgressCallback(void* context, const StartupProgressUpdate& update);

#if defined(BACKROOMS_GAME_EXE)
#include "../game/game_shell.inl"
#endif

#include "../platform/loading_overlay.inl"

#include "../platform/window_proc.inl"

#include "../config/config_dialog.inl"

#if defined(BACKROOMS_GAME_EXE)
#include "../game/game_app.inl"
#else
#include "../screensaver/screensaver_app.inl"
#endif
