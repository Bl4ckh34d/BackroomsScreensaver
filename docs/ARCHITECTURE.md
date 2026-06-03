# Backrooms Maze Architecture

This project is currently in transition from a single-file screensaver into a shared screensaver/game codebase. Keep changes staged and reversible: preserve the screensaver, extract game systems as their boundaries become clear, and avoid a large rewrite that makes visual regressions hard to isolate.

## Current Layout

- `src/main.cpp`: thin shared compilation unit, the remaining `Renderer` state/member declarations, and ordered subsystem includes.
- `src/app/app_state.h`: shared app host state and global app pointer.
- `src/app/entry_point.inl`: process entry point that dispatches to the game or screensaver runtime host.
- `src/app/run_mode.h`: screensaver/game launch mode enum shared by command-line parsing and hosts.
- `src/config/config_dialog_mode.h`: shared config-dialog mode enum.
- `src/config/config_dialog.inl`: legacy Win32 settings dialog plus the embedded game/debug settings host.
- `src/config/config_dialog_model.inl`: legacy dialog tab labels, field schema, and mode-specific settings model builders.
- `src/config/settings.h` / `src/config/settings.cpp`: settings data model API, INI defaults/loading/saving, runtime variation helpers, asset lookup, image loading, and profile toggles.
- `src/core/maze_types.h`: public maze primitives such as `Tile` and default maze dimensions.
- `src/core/math_utils.h`: shared scalar, noise, hash, and interpolation helpers.
- `src/debug/effect_debug.inl`: effect debug mode enums, globals, labels, and ranges.
- `src/debug/debug_control_ids.h`: Win32 command IDs for the effect/debug toolbar.
- `src/debug/debug_slice_controls.inl`: effect/debug toolbar creation, title, command, button state, and redraw helpers.
- `src/debug/debug_slice_effect.h`: effect/debug slice enum shared by parser, debug UI, and renderer.
- `src/debug/startup_progress.h`: startup-progress DTO/callback types shared by renderer startup and platform loading UI.
- `src/game/game_app.inl`: `BackroomsMazeGame.exe` host coordinator for window creation, UI setup, autostart, and the message loop.
- `src/game/game_app_window.inl`: game window class registration and launch placement.
- `src/game/game_app_controls.inl`: game-host Win32 control creation and initial UI state.
- `src/game/game_app_autostart.inl`: developer/profile autostart handling for game launches.
- `src/game/game_app_loop.inl`: game message pump and fixed-tick host loop.
- `src/game/game_app_state_fields.inl`: game-only fields included into the shared `App` host state.
- `src/game/game_window_proc.inl`: `BackroomsMazeGame.exe` window procedure.
- `src/game/game_window_proc_helpers.inl`: game-specific resize, paint, cursor, mouse, activation, and command handlers used by the game window procedure.
- `src/game/game_shell.inl`: game shell coordinator for state transitions, renderer startup, and settings callbacks.
- `src/game/game_menu_runtime.inl`: main-menu coordinator over layout, presentation, command, and transition slices.
- `src/game/game_custom_menu_runtime.inl`: custom-game menu coordinator over layout, spec, state, adjustment, and command slices.
- `src/game/game_input_capture.inl`: game cursor capture and input snapshot collection.
- `src/game/game_shell_ids.h`: Win32 command IDs for game menu and custom-game controls.
- `src/game/game_state.h`: game-host state enum.
- `src/game/game_settings_panel.h` / `src/game/game_settings_panel.cpp`: custom in-game settings screen used by `BackroomsMazeGame.exe`, with host callbacks supplied by the game shell.
- `src/game/game_window_settings.h` / `src/game/game_window_settings.cpp`: game-window fullscreen and resolution application helper.
- `src/game/player_camera_movement.inl`: camera/player/autopilot movement code moved out of the root source folder as the first game-domain extraction.
- `src/gameplay/renderer_update.inl`: renderer frame simulation dispatch and mode-specific update branching.
- `src/gameplay/scare_effect_events.inl`: scare source checks, spark/vent events, and transient effect particle updates.
- `src/maze/maze.h` / `src/maze/maze.cpp`: maze generation, open-cell queries, line clear checks, and pathfinding.
- `src/monster/monster_ai.inl`: monster sight, hearing, pathing, goal selection, and head animation.
- `src/platform/loading_overlay.h` / `src/platform/loading_overlay.cpp`: loading overlay window, spinner/progress rendering, and startup-progress callback bridge.
- `src/platform/command_line_mode.h` / `src/platform/command_line_mode.cpp`: screensaver command-line parsing into launch mode and debug-start options.
- `src/platform/gui_controls.h` / `src/platform/gui_controls.cpp`: shared Win32 GUI control helpers.
- `src/platform/window_proc_helpers.inl`: shared Win32 window-procedure helpers.
- `src/render/render_types.h`: shared render/runtime structs and enums.
- `src/render/renderer_dynamic_geometry.inl`: dynamic meshes for doors, monster, sparks, steam, air particles, and vent drops.
- `src/render/renderer_dependencies.h`: renderer public dependency include bundle used by the renderer class shell.
- `src/render/renderer_maze_mesh.inl`: generated static maze geometry, prop placement, lamps, exit, clutter, and decals.
- `src/render/renderer_mesh_loading.inl`: mesh-loading coordinator over asset path/cache helpers, monster mesh loading, static prop mesh loading, and prop-library loading slices.
- `src/render/renderer_overlays.inl`: D3D overlay drawing, HUD, dread meter, and minimap/debug AI map.
- `src/render/renderer_present.inl`: per-frame constant setup, draw submission, postprocess, overlays, and present.
- `src/render/renderer_private_modules.inl`: renderer private implementation include bundle grouped into state, gameplay, resources, static scene, and frame modules.
- `src/render/renderer_shaders.inl`: shader setup coordinator that specializes HLSL text, compiles blobs, creates shader objects, and creates input layouts through focused renderer shader slices.
- `src/render/renderer_textures.inl`: material atlas creation coordinator, with loose-page, runtime upload/custom-menu, and high-res PBR helpers split into focused slices.
- `src/screensaver/screensaver_app.inl`: `BackroomsMaze.scr` fullscreen/preview/diagnostic host coordinator.
- `src/screensaver/screensaver_app_modes.inl`: screensaver run-mode flags, debug globals, window-class registration, and launch placement.
- `src/screensaver/screensaver_app_windows.inl`: screensaver clone windows, debug controls, and loading-overlay setup.
- `src/screensaver/screensaver_app_renderer.inl`: screensaver renderer initialization and loading warmup setup.
- `src/screensaver/screensaver_app_loop.inl`: screensaver warmup and playback message loop.
- `src/screensaver/screensaver_app_state_fields.inl`: screensaver-only fields included into the shared `App` host state.
- `src/screensaver/screensaver_clone_lookup.inl`: screensaver clone-output lookup by window handle.
- `src/screensaver/screensaver_monitor_layout.h` / `src/screensaver/screensaver_monitor_layout.cpp`: monitor enumeration and ordering for multi-display fullscreen playback.
- `src/screensaver/screensaver_quit.inl`: screensaver target shutdown helper.
- `src/screensaver/screensaver_self_test.inl`: screensaver renderer initialization smoke test entry.
- `src/screensaver/screensaver_window_proc.inl`: `BackroomsMaze.scr` window procedure.
- `src/screensaver/screensaver_window_proc_helpers.inl`: screensaver-specific clone resize, cursor, mouse, quit-input, and activation handlers used by the screensaver window procedure.
- `docs/GAME_PLAN.md`: implementation checklist, backlog, and handoff notes for future agents.

`src/main.cpp` is no longer the project monolith, and stable settings, audio, maze, player, game-world, loading-overlay, and settings-panel boundaries now compile as normal translation units. The remaining include-based files are still tightly coupled to `Renderer` and app host state.

## Target Layout

- `src/platform/`: Win32 window creation, message routing, cursor capture, loading overlay, process/restart helpers.
- `src/config/`: INI schema, loading/saving, migration/defaults, and the legacy screensaver configuration dialog.
- `src/render/`: D3D11 renderer, shaders, render targets, meshes, materials, textures, and postprocessing.
- `src/game/`: game states, menus, player controller, HUD, gameplay settings, interaction, stealth/noise, health/stamina, and save/run state.
- `src/debug/`: debug scene, effect slice viewer, model/material browser, maze generator test tools, and gameplay trigger tests.
- `src/maze/`: maze generation, wall/door geometry data, pathing helpers, and reusable navigation queries.
- `src/monster/`: monster AI, sensory model, procedural body/limb animation, and monster-specific rendering data.
- `src/audio/`: audio device, mixer, emitters, ambience loops, footstep material sounds, and gameplay sound events.

## Extraction Rules

- Keep `BackroomsMaze.scr` and `BackroomsMazeGame.exe` building from the same renderer until each extracted subsystem has a clean API.
- Prefer small include-file moves first when a block is tightly coupled to `main.cpp`; convert to `.cpp/.h` once dependencies are explicit.
- Do not mix screensaver autopilot behavior with playable game behavior. Route differences through `RendererRuntimeMode`, `GameState`, or explicit controller classes.
- Update `docs/GAME_PLAN.md` when a milestone moves from planned to implemented.
- Test the game executable after touching only `src/game/` game shell/settings code.
- Test `/selftest` after touching `src/screensaver/`, renderer, config, texture, movement, shared platform/window lifecycle, or shared app state code.

## Near-Term Extraction Order

- Move loading overlay and game window helpers into `src/platform/`.
- Convert the remaining game shell coordinator into a `src/game/game_shell` unit once app-state dependencies are explicit.
- Keep screensaver runtime code in `src/screensaver/` and game runtime code in `src/game/` so target-specific changes stay localized.
- Convert the most stable extracted include files into `.h/.cpp` units once their dependencies are explicit.
- Split `src/render/renderer_maze_mesh.inl` further into geometry primitives, prop placement, wall/floor/ceiling generation, and effect/decal placement.
- Continue splitting the embedded main-scene shader source by family or move HLSL into external shader assets once shader-cache verification is complete.
- Continue shrinking `Renderer` by extracting durable renderer resource ownership and gameplay state into smaller classes.
