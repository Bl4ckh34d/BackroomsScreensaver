#include "app_state.h"
#include "../debug/debug_slice_controls.inl"
#include "../config/config_dialog_mode.h"

void ShowConfig(HWND owner, ConfigDialogMode mode = ConfigDialogMode::Full);
HWND CreateEmbeddedConfig(HWND parent, ConfigDialogMode mode);
#include "../platform/window_proc_helpers.inl"

#if defined(BACKROOMS_GAME_EXE)
#include "../game/game_shell.inl"
#include "../game/game_window_proc_helpers.inl"
#include "../game/game_window_proc.inl"
#else
#include "../screensaver/screensaver_clone_lookup.inl"
#include "../screensaver/screensaver_quit.inl"
#include "../screensaver/screensaver_window_proc_helpers.inl"
#include "../screensaver/screensaver_window_proc.inl"
#endif

#include "../config/config_dialog.inl"

#if defined(BACKROOMS_GAME_EXE)
#include "../game/game_app.inl"
#else
#include "../screensaver/screensaver_app.inl"
#endif
