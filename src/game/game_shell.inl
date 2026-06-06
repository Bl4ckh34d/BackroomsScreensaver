// Game shell, menu state transitions, mouse capture, and input collection.
// Included from main.cpp after App, Renderer, and ConfigDialogMode are declared.

void EnterGamePlay(HWND hwnd);
void EnterGameDebug(HWND hwnd);
void EnterGameSettings(HWND hwnd, ConfigDialogMode mode, GameState returnState);
void EnterGameCustomMenu(HWND hwnd);
void EnterGameSettingsBoard(HWND hwnd);
void ExecuteGameMenuCommand(HWND hwnd, int id);
void ReleaseGameMouse();
void ExitGameCustomMenu(HWND hwnd);
void ExitGameSettingsBoard(HWND hwnd);
int HitTestSettingsBoard(HWND hwnd, POINT p);
void ActivateSettingsBoardCommand(HWND hwnd, int control);

#include "game_custom_menu_runtime.inl"
#include "game_settings_board_runtime.inl"
#include "game_menu_runtime.inl"
#include "game_input_capture.inl"
#include "game_renderer_startup.inl"
#include "game_state_transitions.inl"
#include "game_settings_runtime.inl"
