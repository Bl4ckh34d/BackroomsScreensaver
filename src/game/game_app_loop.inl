int RunGameMessageLoop(App& app, HWND hwnd) {
#include "game_app_loop_setup.inl"
#include "game_app_loop_messages.inl"
#include "game_app_loop_pause.inl"
#include "game_app_loop_main_menu.inl"
#include "game_app_loop_renderer_tick.inl"
#include "game_app_loop_cleanup.inl"
}
