# Backrooms Maze Refactor Plan

This project is a custom single-game C++/Direct3D runtime that is growing from a screensaver into a playable game. The refactor goal is not to rewrite it into a general engine. The goal is to reduce the current `Renderer` god-object coupling until gameplay, rendering, audio, maze/navigation, and app lifecycle can evolve independently.

Keep every step buildable and reversible. The screensaver remains the baseline visual/runtime target while `BackroomsMazeGame.exe` grows into the playable game.

## Current Problem

Most systems are split into `.inl` files, but they are still included into one large `Renderer` class. That makes `Renderer` own or directly coordinate too much:

- Direct3D device resources, shaders, textures, buffers, and presentation.
- Maze generation, static mesh generation, prop placement, decals, and effects.
- Player movement, health, stamina, collectible state, and camera behavior.
- Monster AI, pathing, sensory state, and animation state.
- Audio event timing, emitter setup, and gameplay sound responses.
- Main menu scene transitions and game/runtime mode switching.
- Debug overlays and effect viewer behavior.

This was acceptable for a visual-first screensaver/prototype. It becomes costly for a game because gameplay changes can accidentally break rendering, screensaver behavior, or startup flow.

## Refactor Principles

- Preserve behavior first. Do not combine refactors with visual redesigns, AI changes, or gameplay tuning unless the extraction requires it.
- Extract by ownership, not by file size alone.
- Prefer small compile-clean slices over a large architectural rewrite.
- Keep `BackroomsMaze.scr` and `BackroomsMazeGame.exe` sharing the same renderer until the replacement interfaces are stable.
- Keep screensaver autopilot behavior separate from playable game behavior through explicit runtime modes or controller APIs.
- Convert include-based modules into `.h/.cpp` units only after their dependencies are clear.
- Add focused tests or smoke checks around each moved boundary.

## Target Ownership

### App / Platform

Owns process entry, window creation, message routing, cursor capture, loading overlay, and target-specific lifecycle.

Candidate files:

- `src/app/`
- `src/platform/`
- `src/screensaver/`
- `src/game/game_app.inl`
- `src/game/game_shell.inl`

### Game World

Owns gameplay state independent of D3D: player state, monster state, collectibles, death/exit transitions, active gameplay events, and per-frame simulation state.

New candidate files:

- `src/game/game_world.h`
- `src/game/game_world.cpp`
- `src/game/player_controller.h`
- `src/game/player_controller.cpp`

### Renderer

Owns GPU resources, render targets, shaders, mesh buffers, draw submission, overlays, and visual-only caches. It should render a world snapshot rather than own the world.

Candidate files:

- `src/render/renderer.h`
- `src/render/renderer.cpp`
- `src/render/renderer_resources.cpp`
- `src/render/renderer_present.cpp`
- `src/render/renderer_overlays.cpp`

### Maze / Navigation

Owns maze generation, open-cell queries, line-of-sight/grid queries, pathfinding, and reusable navigation helpers.

Candidate files:

- `src/maze/maze.h`
- `src/maze/maze.cpp`
- `src/maze/navigation.h`
- `src/maze/navigation.cpp`

### Audio

Owns XAudio resources, loaded samples, persistent emitters, and playback. Gameplay should submit sound events instead of directly mutating renderer-owned audio timers.

Candidate files:

- `src/audio/audio_engine.h`
- `src/audio/audio_engine.cpp`
- `src/audio/game_audio_system.h`
- `src/audio/game_audio_system.cpp`

### Monster

Owns monster decision-making, sensory model, search/chase state, and body/limb simulation data that is not purely render geometry.

Candidate files:

- `src/monster/monster_ai.h`
- `src/monster/monster_ai.cpp`
- `src/monster/monster_state.h`
- `src/monster/monster_state.cpp`

## Tracking Rules

- Update checkboxes in this file in the same change that completes the task.
- Add a short note under the relevant phase when a task is intentionally skipped, deferred, or changed.
- Keep each checked task backed by a successful build or an explicit note that verification was not possible.
- Prefer one phase or subphase per agent handoff. Do not mix unrelated gameplay tuning with refactor tasks.
- Leave `BackroomsMaze_backup.ini` unstaged unless it has an intentional content change.

## Standard Verification Commands

Use these commands after any refactor that touches shared runtime, renderer, config, maze, audio, input, or app lifecycle code:

```powershell
cmake --build build --config Release --parallel
Copy-Item .\build\Release\BackroomsMaze.scr .\build\Release\BackroomsMazeTest.exe -Force
Start-Process .\build\Release\BackroomsMazeTest.exe -ArgumentList "/selftest" -Wait -PassThru
Start-Process .\build\Release\BackroomsMazeGame.exe
```

Manual game smoke path:

- Main menu renders.
- Single Player starts a playable maze.
- WASD and mouse look work.
- Sprint, crouch, jump, flashlight, and interact still respond.
- Escape returns to menu.
- Settings opens, saves, and returns.
- Debug opens and returns.
- Exit path closes cleanly.

## Phase 0: Baseline Safety Checklist

Goal: make sure future agents know the starting point and can compare behavior after each slice.

- [ ] Record current branch name and dirty worktree summary before starting a refactor slice.
- [ ] Confirm existing uncommitted changes are understood or unrelated.
- [ ] Build Release successfully for `BackroomsMaze` and `BackroomsMazeGame`.
- [ ] Run `/selftest` through the copied `.exe` helper.
- [ ] Smoke-test `BackroomsMazeGame.exe` main menu.
- [ ] Smoke-test Single Player movement and escape-to-menu.
- [ ] Smoke-test Settings open/save/back flow.
- [ ] Smoke-test Debug open/back flow.
- [ ] Note baseline performance or obvious visual quirks if they exist before refactor work.

Completion notes:

- Agent:
- Date:
- Build result:
- Selftest result:
- Game smoke result:
- Known pre-existing issues:

## Phase 1: Make Data Boundaries Visible

Goal: identify ownership inside `Renderer` before moving behavior.

### 1.1 State Inventory

- [ ] Add ownership section comments in `src/render/renderer_state.inl` for GPU resources.
- [ ] Add ownership section comments in `src/render/renderer_state.inl` for render caches and generated geometry.
- [ ] Add ownership section comments in `src/render/renderer_state.inl` for menu scene state.
- [ ] Add ownership section comments in `src/render/renderer_state.inl` for player/gameplay state.
- [ ] Add ownership section comments in `src/render/renderer_state.inl` for monster state.
- [ ] Add ownership section comments in `src/render/renderer_state.inl` for audio state.
- [ ] Add ownership section comments in `src/render/renderer_state.inl` for debug/effect state.
- [ ] Document any fields whose ownership is unclear.

### 1.2 Snapshot Boundary

- [ ] Review `src/gameplay/playable_snapshot.inl` and identify what already serves as a render-facing snapshot.
- [ ] Decide whether to extend the existing snapshot or create `GameWorldSnapshot`.
- [ ] Add a renderer-facing snapshot struct without moving behavior yet.
- [ ] Include player camera data in the snapshot.
- [ ] Include player health/stamina/interaction HUD data in the snapshot.
- [ ] Include monster render inputs in the snapshot.
- [ ] Include collectible/page HUD data in the snapshot.
- [ ] Keep existing call sites working with no behavior change.

### 1.3 Accessor Cleanup

- [ ] Add helper accessors for player camera/state reads that are used across render, audio, and gameplay.
- [ ] Add helper accessors for monster position/sensory reads that are used across render, audio, and gameplay.
- [ ] Add helper accessors for current runtime mode checks where repeated branching obscures intent.
- [ ] Replace only low-risk direct field reads with accessors.
- [ ] Avoid moving data between owners in this phase.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] No intentional visual or gameplay behavior change.

Completion notes:

- Agent:
- Date:
- Files touched:
- Deferred items:

## Phase 2: Extract Maze and Navigation Interfaces

Goal: make maze/pathing reusable without depending on renderer internals.

### 2.1 Maze Header Split

- [ ] Create `src/maze/maze.h`.
- [ ] Move `Tile` dependency decision into a stable header location.
- [ ] Move `Maze` declarations into `maze.h`.
- [ ] Keep implementation temporarily in `maze.inl` if needed for a small first slice.
- [ ] Update include sites to use `maze.h` where possible.
- [ ] Confirm no renderer-specific types leak into the public maze API.

### 2.2 Maze Implementation Split

- [ ] Create `src/maze/maze.cpp`.
- [ ] Move maze generation implementation into `maze.cpp`.
- [ ] Move open-cell and bounds query implementation into `maze.cpp`.
- [ ] Move line-clear query implementation into `maze.cpp`.
- [ ] Move pathfinding implementation into `maze.cpp`.
- [ ] Update `CMakeLists.txt` to compile `maze.cpp`.
- [ ] Remove obsolete include-only maze implementation once compile-clean.

### 2.3 Navigation Helpers

- [ ] Create `src/maze/navigation.h` if helper APIs are broader than `Maze`.
- [ ] Create `src/maze/navigation.cpp` if implementation is non-trivial.
- [ ] Move tile/world conversion helpers to a shared location.
- [ ] Move neighbor iteration helpers to maze/navigation.
- [ ] Move renderer-independent path scoring helpers out of player movement or monster AI.
- [ ] Keep audio occlusion grid queries using the same maze/navigation API.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Maze generation appears deterministic versus baseline.
- [ ] Monster pathing still works.
- [ ] Audio occlusion still responds to walls.

Completion notes:

- Agent:
- Date:
- Public APIs added:
- Files removed or deprecated:
- Behavior changes:

## Phase 3: Extract Player Controller

Goal: remove playable player state and manual movement logic from renderer ownership.

### 3.1 Player Data Model

- [ ] Create `src/game/player_state.h`.
- [ ] Add `PlayerState` position fields.
- [ ] Add `PlayerState` yaw/pitch/body yaw fields.
- [ ] Add `PlayerState` health and stamina fields.
- [ ] Add `PlayerState` vertical movement/jump/crouch fields.
- [ ] Add `PlayerState` interaction and flashlight fields.
- [ ] Add `PlayerState` visited-tile or exploration fields if they are gameplay-owned.
- [ ] Keep render-only camera shake and postprocess fields out of `PlayerState` unless needed by gameplay.

### 3.2 Player Controller Shell

- [ ] Create `src/game/player_controller.h`.
- [ ] Create `src/game/player_controller.cpp` or temporary `.inl` shell if dependency cleanup requires it.
- [ ] Define `PlayerController::Update` inputs: `GameInputSnapshot`, `Settings`, `Maze`/navigation, `dt`.
- [ ] Define `PlayerController::Update` outputs: updated `PlayerState`, player sound/noise events, interaction attempts.
- [ ] Move low-risk stamina update logic first.
- [ ] Move low-risk crouch height/speed logic.
- [ ] Move jump/gravity logic.
- [ ] Move collision-aware horizontal movement.
- [ ] Move manual mouse-look smoothing and pitch limits.
- [ ] Keep screensaver autopilot code separate from manual player controller.

### 3.3 Renderer Integration

- [ ] Replace renderer-owned player health/stamina fields with `PlayerState` access where possible.
- [ ] Feed camera render data from `PlayerState`.
- [ ] Feed HUD health/stamina from `PlayerState`.
- [ ] Feed player noise pulses from controller output instead of renderer side effects.
- [ ] Preserve flashlight visual presentation in renderer until render/gameplay split is ready.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] WASD movement feels unchanged.
- [ ] Mouse look feels unchanged.
- [ ] Sprint stamina drain/regeneration works.
- [ ] Crouch height/speed works.
- [ ] Jump/gravity works.
- [ ] Exit interaction still works.
- [ ] Screensaver autopilot behavior still works.

Completion notes:

- Agent:
- Date:
- Player fields moved:
- Player fields intentionally left in renderer:
- Behavior changes:

## Phase 4: Extract Monster AI State

Goal: separate monster decisions from monster rendering.

### 4.1 Monster Data Model

- [ ] Create `src/monster/monster_state.h`.
- [ ] Add monster position and yaw fields.
- [ ] Add path, path index, and goal tile fields.
- [ ] Add sound target and last-known target fields.
- [ ] Add roam/search/chase state fields.
- [ ] Add recognition and curiosity timers.
- [ ] Add sensory flags for seen/heard player.
- [ ] Decide whether body/limb simulation is gameplay-owned or render-owned.

### 4.2 Monster AI API

- [ ] Create `src/monster/monster_ai.h`.
- [ ] Create `src/monster/monster_ai.cpp` or temporary `.inl` shell if needed.
- [ ] Define update inputs: `Maze`/navigation, player state, sound pulses, settings, `dt`.
- [ ] Define update outputs: updated `MonsterState`, scare/death/gameplay events.
- [ ] Move target selection logic.
- [ ] Move sight/hearing checks.
- [ ] Move search/chase/roam transitions.
- [ ] Move monster path refresh logic.
- [ ] Keep render-only eye lights, shadow setup, dynamic geometry, and buffers in renderer.

### 4.3 Renderer Integration

- [ ] Read monster position from `MonsterState`.
- [ ] Read monster sensory flags from `MonsterState`.
- [ ] Read monster path/debug data from `MonsterState`.
- [ ] Keep monster mesh/skull loading in renderer.
- [ ] Keep monster shadow resources in renderer.
- [ ] Keep monster dynamic vertex generation in renderer until a visual-animation boundary exists.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Monster can still roam/search/chase.
- [ ] Monster can still react to player sound.
- [ ] Debug AI minimap still shows monster goal/path/sound target.
- [ ] Monster visual rendering remains intact.

Completion notes:

- Agent:
- Date:
- Monster fields moved:
- Monster fields intentionally left in renderer:
- Behavior changes:

## Phase 5: Extract Game World

Goal: centralize gameplay simulation outside renderer.

### 5.1 Game World Shell

- [ ] Create `src/game/game_world.h`.
- [ ] Create `src/game/game_world.cpp` or temporary `.inl` shell if needed.
- [ ] Add `GameWorld` ownership of `Maze`.
- [ ] Add `GameWorld` ownership of `PlayerState`.
- [ ] Add `GameWorld` ownership of `MonsterState`.
- [ ] Add `GameWorld` ownership of collectibles/page state.
- [ ] Add `GameWorld` ownership of death and exit transition state.
- [ ] Add `GameWorld` ownership of gameplay sound pulses/events.

### 5.2 Simulation Update

- [ ] Define `GameWorld::Initialize` inputs currently needed by renderer startup.
- [ ] Define `GameWorld::Update` inputs: input snapshot, settings, `dt`.
- [ ] Move playable-game simulation dispatch out of `renderer_update.inl`.
- [ ] Move death/exit progression that is gameplay-owned.
- [ ] Move collectible/page pickup progression.
- [ ] Move gameplay scare event triggering if it affects game state.
- [ ] Keep visual-only particle updates in renderer until an effects boundary exists.

### 5.3 Render Snapshot

- [ ] Define `GameWorldSnapshot`.
- [ ] Include camera/player render state.
- [ ] Include monster render state.
- [ ] Include maze render references or stable IDs.
- [ ] Include HUD data.
- [ ] Include debug overlay data.
- [ ] Update renderer to consume snapshot for playable game rendering.

### 5.4 Debug Integration

- [ ] Expose debug read-only world state access.
- [ ] Expose debug override hooks only where currently needed.
- [ ] Keep effect slice viewer independent from playable `GameWorld` unless intentionally merged.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Main menu still works.
- [ ] Single Player starts from a clean world.
- [ ] Returning to menu and starting again resets world state correctly.
- [ ] Debug still opens and returns.

Completion notes:

- Agent:
- Date:
- World-owned systems:
- Renderer-owned systems remaining:
- Behavior changes:

## Phase 6: Extract Audio Event Flow

Goal: stop gameplay audio from being renderer-owned timing logic.

### 6.1 Audio Event Types

- [ ] Create `src/audio/game_audio_events.h`.
- [ ] Define event type for footsteps.
- [ ] Define event type for door open/close/lock.
- [ ] Define event type for lamp buzz/flicker/break.
- [ ] Define event type for vent puffs/groans.
- [ ] Define event type for monster vocals.
- [ ] Define event type for wet drips/water effects.
- [ ] Define event type for scare/effect stingers.
- [ ] Include position, volume, pitch, bus, spatialization, and occlusion hints as needed.

### 6.2 Game Audio System

- [ ] Create `src/audio/game_audio_system.h`.
- [ ] Create `src/audio/game_audio_system.cpp` or temporary `.inl` shell if needed.
- [ ] Move delayed audio event scheduling out of renderer.
- [ ] Move footstep audio triggering out of renderer.
- [ ] Move monster vocal timing out of renderer if gameplay-owned.
- [ ] Move door audio transition logic out of renderer if gameplay-owned.
- [ ] Keep low-level `AudioEngine` sample loading and playback separate.

### 6.3 Monster Hearing Integration

- [ ] Make player movement emit sound/noise events.
- [ ] Make loud world effects emit sound/noise events.
- [ ] Feed monster hearing from sound/noise events instead of renderer-owned pulse fields.
- [ ] Keep audio occlusion queries routed through maze/navigation APIs.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Footsteps still play.
- [ ] Door sounds still play.
- [ ] Lamp/vent/drip sounds still play.
- [ ] Monster vocals still play.
- [ ] Monster still reacts to player noise.

Completion notes:

- Agent:
- Date:
- Audio logic moved:
- Audio logic left in renderer:
- Behavior changes:

## Phase 7: Split Static Maze Rendering

Goal: reduce `renderer_maze_mesh.inl` into smaller render-building modules.

### 7.1 Preparation

- [ ] Add section comments to `renderer_maze_mesh.inl` marking geometry primitives.
- [ ] Mark wall/floor/ceiling mesh generation.
- [ ] Mark prop placement.
- [ ] Mark lamp placement.
- [ ] Mark decals/damage/effects placement.
- [ ] Mark exit geometry.
- [ ] Mark final buffer/index upload logic.
- [ ] Identify shared helper functions and local structs.

### 7.2 File Splits

- [ ] Create `src/render/renderer_geometry_primitives.inl` or `.cpp` when dependencies allow.
- [ ] Create `src/render/renderer_wall_floor_ceiling_mesh.inl` or `.cpp`.
- [ ] Create `src/render/renderer_prop_placement.inl` or `.cpp`.
- [ ] Create `src/render/renderer_lamp_placement.inl` or `.cpp`.
- [ ] Create `src/render/renderer_decals_and_damage.inl` or `.cpp`.
- [ ] Create `src/render/renderer_exit_geometry.inl` or `.cpp`.
- [ ] Create `src/render/renderer_static_mesh_upload.inl` or `.cpp`.
- [ ] Update `renderer.inl` include order or CMake sources accordingly.

### 7.3 Determinism Checks

- [ ] Confirm maze seed produces the same base layout.
- [ ] Confirm prop placement is not unintentionally changed.
- [ ] Confirm lamp placement is not unintentionally changed.
- [ ] Confirm exit geometry is not unintentionally changed.
- [ ] Confirm water/blood/damage decal placement is not unintentionally changed.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] No unexpected visual differences in main menu.
- [ ] No unexpected visual differences in playable maze.
- [ ] Debug/effect viewer still renders.

Completion notes:

- Agent:
- Date:
- Files split:
- Determinism notes:
- Behavior changes:

## Phase 8: Split Shaders

Goal: make shader work manageable without searching one multi-thousand-line embedded string file.

### 8.1 Strategy Decision

- [ ] Decide between embedded per-family shader strings and external HLSL assets.
- [ ] If external HLSL is chosen, update package/build scripts to copy or embed shader assets.
- [ ] If embedded strings are kept, define one source file per shader family.
- [ ] Document shader cache hash/version implications.

### 8.2 Shader Family Split

- [ ] Split maze/material shader.
- [ ] Split dynamic object shader.
- [ ] Split liquid/effects shader.
- [ ] Split overlay shader.
- [ ] Split textured overlay shader.
- [ ] Split postprocess shader.
- [ ] Split shadow shaders.
- [ ] Keep shader entry point names stable.
- [ ] Keep input layout creation behavior stable.

### 8.3 Cache and Packaging

- [ ] Confirm shader cache file naming remains correct.
- [ ] Bump shader cache version only if compiled source or cache assumptions changed.
- [ ] Confirm cache miss compile path works.
- [ ] Confirm cache hit load path works.
- [ ] Confirm release packaging includes any new shader files if applicable.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Shader cold-start path works after clearing relevant cache files.
- [ ] Shader warm-start cached path works.
- [ ] Main menu, playable game, overlays, shadows, and postprocess all render.

Completion notes:

- Agent:
- Date:
- Strategy chosen:
- Cache/version changes:
- Packaging changes:

## Phase 9: Convert Stable `.inl` Modules

Goal: replace include-based implementation units with normal C++ translation units once dependencies are explicit.

### 9.1 Low-Risk Candidates

- [ ] Convert `src/audio/audio_engine.inl` to `.h/.cpp`.
- [ ] Convert `src/maze/maze.inl` fully if not already done.
- [ ] Convert renderer-independent parts of `src/config/settings.inl`.
- [ ] Convert stable settings model code from `src/config/config_dialog_model.inl` if dependencies allow.

### 9.2 Game/App Candidates

- [ ] Convert `src/game/game_settings_panel.inl` once UI dependencies are explicit.
- [ ] Convert `src/game/game_shell.inl` once app state dependencies are explicit.
- [ ] Convert `src/game/game_app.inl` once platform dependencies are explicit.
- [ ] Convert `src/screensaver/screensaver_app.inl` only after screensaver smoke coverage is reliable.

### 9.3 Renderer Candidates

- [ ] Convert renderer submodules only after their private interfaces are clear.
- [ ] Avoid exposing D3D internals through broad public headers.
- [ ] Keep renderer public API small: initialize, resize, update/render, runtime mode, snapshots/debug hooks.
- [ ] Remove obsolete `.inl` includes from `renderer.inl` as modules become real translation units.
- [ ] Update `CMakeLists.txt` for each new `.cpp`.

Verification:

- [ ] Release build passes.
- [ ] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Compile errors point to module boundaries rather than include order.
- [ ] `src/main.cpp` remains a thin entry compile shell or becomes a normal entry translation unit.

Completion notes:

- Agent:
- Date:
- Modules converted:
- Include-order risks removed:
- Remaining `.inl` modules:

## Risk Areas

- Screensaver `/s`, `/p <HWND>`, `/c`, and `/selftest` behavior.
- Settings persistence and INI default merging.
- Shader and texture cache invalidation.
- Maze determinism and prop placement determinism.
- Runtime mode differences between screensaver autopilot, main menu, playable game, debug viewer, and preview.
- Audio initialization and cleanup order.
- Win32 message routing and mouse capture.

## Done Definition

The refactor is successful when:

- `Renderer` owns rendering, not gameplay progression.
- Player and monster simulation can be reasoned about without Direct3D state.
- Maze/navigation APIs are reusable by player, monster, audio occlusion, and debug tools.
- Audio consumes explicit events instead of hidden renderer-side gameplay state.
- The screensaver and game targets both build and pass smoke checks.
- New gameplay features no longer require edits across renderer, audio, monster, and app lifecycle files for routine behavior.
