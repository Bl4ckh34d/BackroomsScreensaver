# Backrooms Maze Architecture

This project is currently in transition from a single-file screensaver into a shared screensaver/game codebase. Keep changes staged and reversible: preserve the screensaver, extract game systems as their boundaries become clear, and avoid a large rewrite that makes visual regressions hard to isolate.

## Current Layout

- `src/main.cpp`: thin shared compilation unit, the remaining `Renderer` state/member declarations, and ordered subsystem includes.
- `src/app/app_state.inl`: shared app state, run modes, game state enum, command IDs, and debug toolbar helpers.
- `src/app/entry_point.inl`: process entry point that dispatches to the game or screensaver runtime host.
- `src/config/config_dialog.inl`: legacy Win32 settings dialog plus the embedded game/debug settings host.
- `src/config/config_dialog_model.inl`: legacy dialog tab labels, field schema, and mode-specific settings model builders.
- `src/config/settings.inl`: INI defaults, loading/saving, runtime variation helpers, asset lookup, and image loading.
- `src/core/math_utils.inl`: shared scalar, noise, hash, and tile comparison helpers.
- `src/debug/effect_debug.inl`: effect debug mode enums, globals, labels, ranges, and startup-progress helpers.
- `src/game/game_app.inl`: `BackroomsMazeGame.exe` window creation and main loop.
- `src/game/game_shell.inl`: game menu layout, state transitions, mouse capture, game renderer startup, and input collection.
- `src/game/game_settings_panel.inl`: custom in-game settings screen used by `BackroomsMazeGame.exe`.
- `src/game/player_camera_movement.inl`: camera/player/autopilot movement code moved out of the root source folder as the first game-domain extraction.
- `src/gameplay/renderer_update.inl`: renderer frame simulation dispatch and mode-specific update branching.
- `src/gameplay/scare_effect_events.inl`: scare source checks, spark/vent events, and transient effect particle updates.
- `src/maze/maze.inl`: maze generation, open-cell queries, line clear checks, and pathfinding.
- `src/monster/monster_ai.inl`: monster sight, hearing, pathing, goal selection, and head animation.
- `src/platform/loading_overlay.inl`: loading overlay window, spinner/progress rendering, and startup-progress callback bridge.
- `src/platform/window_proc.inl`: shared Win32 window procedure, screensaver quit handling, and command-line mode parsing.
- `src/render/render_types.inl`: shared render/runtime structs and enums.
- `src/render/renderer_dynamic_geometry.inl`: dynamic meshes for doors, monster, sparks, steam, air particles, and vent drops.
- `src/render/renderer_maze_mesh.inl`: generated static maze geometry, prop placement, lamps, exit, clutter, and decals.
- `src/render/renderer_mesh_loading.inl`: external OBJ/static prop mesh loading and cache helpers.
- `src/render/renderer_overlays.inl`: D3D overlay drawing, HUD, dread meter, and minimap/debug AI map.
- `src/render/renderer_present.inl`: per-frame constant setup, draw submission, postprocess, overlays, and present.
- `src/render/renderer_shaders.inl`: embedded HLSL shader sources, compilation, and input layouts.
- `src/render/renderer_textures.inl`: material atlas creation and texture upload/cache helpers.
- `src/screensaver/screensaver_app.inl`: `BackroomsMaze.scr` fullscreen/preview/diagnostic host and `/selftest`.
- `docs/GAME_PLAN.md`: implementation checklist, backlog, and handoff notes for future agents.

`src/main.cpp` is no longer the project monolith, but the extracted files are still include-based and tightly coupled to `Renderer`. The next architectural step is turning stable include boundaries into normal `.h/.cpp` units with explicit interfaces.

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
- Move the game state/menu functions into a `src/game/game_shell` unit.
- Keep screensaver runtime code in `src/screensaver/` and game runtime code in `src/game/` so target-specific changes stay localized.
- Convert the most stable extracted include files into `.h/.cpp` units once their dependencies are explicit.
- Split `src/render/renderer_maze_mesh.inl` further into geometry primitives, prop placement, wall/floor/ceiling generation, and effect/decal placement.
- Split `src/render/renderer_shaders.inl` by shader family or move HLSL into external shader assets.
- Continue shrinking `Renderer` by extracting durable renderer resource ownership and gameplay state into smaller classes.
