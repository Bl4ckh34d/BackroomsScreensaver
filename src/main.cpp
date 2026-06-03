#include "platform/platform_headers.h"

#include "core/maze_types.h"

#include "core/math_utils.h"

#include "core/constants.h"

#include "debug/effect_debug_constants.h"

#include "config/settings.h"

#include "debug/effect_debug.h"

#include "debug/debug_control_ids.h"

#include "platform/loading_overlay.h"
#include "platform/command_line_mode.h"
#include "platform/gui_controls.h"

#if !defined(BACKROOMS_GAME_EXE)
#include "screensaver/screensaver_monitor_layout.h"
#endif

#if defined(BACKROOMS_GAME_EXE)
#include "game/game_state.h"
#include "game/game_shell_ids.h"
#endif

#include "audio/audio_engine.h"

#include "audio/game_audio_events.h"

#include "audio/game_audio_system.h"

#include "maze/maze.h"

#include "game/game_input.h"

#if defined(BACKROOMS_GAME_EXE)
#include "game/game_settings_panel.h"
#include "game/game_window_settings.h"
#endif

#include "game/runtime_mode.h"

#include "game/player_controller.h"

#include "game/game_world_types.h"

#include "monster/monster_state.h"

#include "gameplay/playable_progression_types.h"

#include "game/game_session.h"

#include "game/game_world.h"

namespace {

#include "render/renderer.inl"

#include "app/app_runtime.inl"

} // namespace

#include "app/entry_point.inl"
