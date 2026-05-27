# Backrooms Maze Architecture

This project is currently in transition from a single-file screensaver into a shared screensaver/game codebase. Keep changes staged and reversible: preserve the screensaver, extract game systems as their boundaries become clear, and avoid a large rewrite that makes visual regressions hard to isolate.

## Current Layout

- `src/main.cpp`: legacy application host, Win32 entry points, renderer, config dialog, game shell glue, and remaining screensaver systems.
- `src/game/game_settings_panel.inl`: custom in-game settings screen used by `BackroomsMazeGame.exe`.
- `src/game/player_camera_movement.inl`: camera/player/autopilot movement code moved out of the root source folder as the first game-domain extraction.
- `docs/GAME_PLAN.md`: implementation checklist, backlog, and handoff notes for future agents.

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
- Test both the game executable and `/selftest` after touching renderer, config, texture, movement, or window lifecycle code.

## Near-Term Extraction Order

- Move loading overlay and game window helpers into `src/platform/`.
- Move the game state/menu functions into a `src/game/game_shell` unit.
- Split the legacy config dialog from config parsing and defaults.
- Move maze generation/path helpers into `src/maze/` after the player and monster APIs stop reaching into renderer internals directly.
- Extract renderer resources last, after gameplay and platform dependencies are no longer interleaved with render setup.
