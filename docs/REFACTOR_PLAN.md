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

## Execution Adjustment

The current codebase already makes the ownership problem visible: the `.inl` files are grouped by domain, but most are still included inside `Renderer`. Keep Phase 1 brief and do not let ownership comments block extraction work.

Near-term execution order:

- First extract the `Maze` public API into `maze.h` while keeping implementation include-based if that is the smallest compile-clean slice.
- Treat `BackroomsMazeGame.exe` as the primary game host. Treat `BackroomsMaze.scr` as an appendix host that starts a game-like run with autopilot player control and screensaver-specific presentation differences.
- Keep host state split by target so screensaver preview/fullscreen/clone handling does not leak into the game host, and game menu/input/settings-shell state does not leak into the screensaver host.
- Treat `PlayableSnapshot` as pause/restore state, not the future render-facing world snapshot.
- Introduce `PlayerState` before moving controller behavior.
- Introduce `MonsterState` before moving AI behavior.
- Move gameplay audio through explicit events after player and monster state boundaries exist.

## Target Ownership

### App / Platform

Owns process entry, window creation, message routing, cursor capture, loading overlay, and target-specific lifecycle.

Execution note:

- `BackroomsMazeGame.exe` should remain the primary interactive runtime host.
- `BackroomsMaze.scr` should be a thin appendix host over game/runtime systems: fullscreen/preview/selftest/window-management plus autopilot and screensaver-specific overlay behavior.
- Shared platform helpers are allowed, but host state and window-message behavior should be target-specific.

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
- `src/game/game_world_types.h`
- `src/game/runtime_mode.h`
- `src/game/game_session.cpp`
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

- [x] Record current branch name and dirty worktree summary before starting a refactor slice.
- [x] Confirm existing uncommitted changes are understood or unrelated.
- [x] Build Release successfully for `BackroomsMaze` and `BackroomsMazeGame`.
- [x] Run `/selftest` through the copied `.exe` helper.
- [ ] Smoke-test `BackroomsMazeGame.exe` main menu.
- [ ] Smoke-test Single Player movement and escape-to-menu.
- [ ] Smoke-test Settings open/save/back flow.
- [ ] Smoke-test Debug open/back flow.
- [ ] Note baseline performance or obvious visual quirks if they exist before refactor work.

Completion notes:

- Agent: Codex
- Date: 2026-06-03
- Build result: Release builds passed for `BackroomsMaze.scr` and `BackroomsMazeGame.exe` during the render split pass.
- Selftest result: `BackroomsMazeTest.exe /selftest` returned exit code 0 after the render split pass.
- Game smoke result:
- Known pre-existing issues: dirty worktree contains ongoing refactor changes and unrelated untracked `assets/PBRs/downloads/OfficeCeiling001_4K-JPG.zip`.

Host decoupling notes:

- Agent: Codex
- Date: 2026-06-02
- Completed: split `App` host state by target, so the game target no longer carries screensaver clone/quit/mouse-sentinel state and the screensaver target no longer carries game menu/input/settings-shell state.
- Completed: renamed the main window procedures to target-specific `GameWndProc` and `ScreensaverWndProc` and registered each host with its own procedure.
- Completed: added an explicit `GameSessionSpec`/`StartGameSession` boundary. Game runs now launch as manual, progression-enabled sessions; screensaver output explicitly starts an autopilot, progression-disabled appendix session with screensaver minimap style.
- Verification: Release build passed for `BackroomsMaze.scr` and `BackroomsMazeGame.exe`; `BackroomsMazeTest.exe /selftest` returned exit code 0.

## Phase 1: Make Data Boundaries Visible

Goal: identify ownership inside `Renderer` before moving behavior.

### 1.1 State Inventory

- [x] Add ownership section comments in `src/render/renderer_state.inl` for GPU resources.
- [x] Add ownership section comments in `src/render/renderer_state.inl` for render caches and generated geometry.
- [x] Add ownership section comments in `src/render/renderer_state.inl` for menu scene state.
- [x] Add ownership section comments in `src/render/renderer_state.inl` for player/gameplay state.
- [x] Add ownership section comments in `src/render/renderer_state.inl` for monster state.
- [x] Add ownership section comments in `src/render/renderer_state.inl` for audio state.
- [x] Add ownership section comments in `src/render/renderer_state.inl` for debug/effect state.
- [x] Document any fields whose ownership is unclear.

### 1.2 Snapshot Boundary

- [x] Review `src/gameplay/playable_snapshot.inl` and identify what already serves as a render-facing snapshot.
- [x] Decide whether to extend the existing snapshot or create `GameWorldSnapshot`.
- [x] Add a renderer-facing snapshot struct without moving behavior yet.
- [x] Include player camera data in the snapshot.
- [x] Include player health/stamina/interaction HUD data in the snapshot.
- [x] Include monster render inputs in the snapshot.
- [x] Include collectible/page HUD data in the snapshot.
- [x] Keep existing call sites working with no behavior change.

### 1.3 Accessor Cleanup

- [ ] Add helper accessors for player camera/state reads that are used across render, audio, and gameplay.
- [ ] Add helper accessors for monster position/sensory reads that are used across render, audio, and gameplay.
- [x] Add helper accessors for current runtime mode checks where repeated branching obscures intent.
- [x] Replace only low-risk direct field reads with accessors.
- [ ] Avoid moving data between owners in this phase.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
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

- [x] Create `src/maze/maze.h`.
- [x] Move `Tile` dependency decision into a stable header location.
- [x] Move `Maze` declarations into `maze.h`.
- [x] Keep implementation temporarily in `maze.inl` if needed for a small first slice.
- [x] Update include sites to use `maze.h` where possible.
- [x] Confirm no renderer-specific types leak into the public maze API.
- [x] Replace full `Settings` dependencies in maze generation with `MazeGenerationSpec` and `MazeLayoutSpec` adapters.

### 2.2 Maze Implementation Split

- [x] Create `src/maze/maze.cpp`.
- [x] Move maze generation implementation into `maze.cpp`.
- [x] Move open-cell and bounds query implementation into `maze.cpp`.
- [x] Move line-clear query implementation into `maze.cpp`.
- [x] Move pathfinding implementation into `maze.cpp`.
- [x] Update `CMakeLists.txt` to compile `maze.cpp`.
- [x] Remove obsolete include-only maze implementation once compile-clean.

Notes:

- `Maze` no longer accepts the broad app `Settings` object for generation. `src/maze/maze_settings_adapter.h` maps runtime settings into maze-only generation/layout specs, and `GameWorld` applies maze layout through `MazeLayoutSpec`.
- `Maze` now builds from `src/maze/maze.cpp`. `Tile` plus maze default constants live in `src/core/maze_types.h`, so maze/navigation code no longer depends on renderer-local type declarations.

### 2.3 Navigation Helpers

- [ ] Create `src/maze/navigation.h` if helper APIs are broader than `Maze`.
- [ ] Create `src/maze/navigation.cpp` if implementation is non-trivial.
- [ ] Move tile/world conversion helpers to a shared location.
- [ ] Move neighbor iteration helpers to maze/navigation.
- [ ] Move renderer-independent path scoring helpers out of player movement or monster AI.
- [ ] Keep audio occlusion grid queries using the same maze/navigation API.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Maze generation appears deterministic versus baseline.
- [ ] Monster pathing still works.
- [ ] Audio occlusion still responds to walls.

Completion notes:

- Agent: Codex
- Date: 2026-06-02
- Public APIs added: `src/maze/maze.h` now owns `MazeWallFeature` and `Maze` declarations.
- Files removed or deprecated: `src/maze/maze.inl` was removed after `src/maze/maze.cpp` was added to CMake.
- Behavior changes: none intended. Release build passed for `BackroomsMaze.scr` and `BackroomsMazeGame.exe`. `/selftest` helper completed successfully from PowerShell; child process `ExitCode` was not populated in the returned process object.

## Phase 3: Extract Player Controller

Goal: remove playable player state and manual movement logic from renderer ownership.

### 3.1 Player Data Model

- [x] Create `src/game/player_state.h`.
- [x] Create `src/game/player_state.cpp`.
- [x] Add `PlayerState` position fields.
- [x] Add `PlayerState` yaw/pitch/body yaw fields.
- [x] Add `PlayerState` health and stamina fields.
- [x] Add `PlayerState` vertical movement/jump/crouch fields.
- [x] Add `PlayerState` interaction and flashlight fields.
- [x] Add `PlayerState` visited-tile or exploration fields if they are gameplay-owned.
- [x] Keep render-only camera shake and postprocess fields out of `PlayerState` unless needed by gameplay.

### 3.2 Player Controller Shell

- [x] Create `src/game/player_controller.h`.
- [x] Create `src/game/player_controller.cpp` or temporary `.inl` shell if dependency cleanup requires it.
- [x] Define `PlayerController::Update` inputs: `GameInputSnapshot`, movement tuning, service callbacks, and `dt`.
- [ ] Define `PlayerController::Update` outputs: updated `PlayerState`, player sound/noise events, interaction attempts.
- [x] Define manual-control update input/output shell.
- [x] Move low-risk stamina update logic first.
- [x] Move low-risk crouch height/speed logic.
- [ ] Move jump/gravity logic.
- [x] Move player input latch logic.
- [x] Split player interaction helpers out of the camera/movement file.
- [x] Move manual horizontal movement orchestration.
- [x] Add controller-friendly collision move request interface.
- [x] Split renderer player/camera helpers into temporary movement, collision, attention, pathing, manual, and autopilot `.inl` slices.
- [x] Move renderer maze/camera collision implementation behind a world/collision service.
- [x] Move manual mouse-look smoothing and pitch limits.
- [x] Move player step-phase advancement math.
- [x] Move manual camera height/head-bob composition.
- [x] Keep screensaver autopilot code separate from manual player controller.
- [x] Move `GameInputSnapshot` out of renderer types into `src/game/game_input.h`.
- [x] Move shared scalar/noise math helpers from include-only `src/core/math_utils.inl` to `src/core/math_utils.h`.
- [x] Move non-template `PlayerController` methods from `src/game/player_controller.inl` to `src/game/player_controller.cpp`.
- [x] Move `PlayerState` session/snapshot/vitals methods from header bodies to `src/game/player_state.cpp`.

### 3.3 Renderer Integration

- [x] Replace renderer-owned player health/stamina fields with `PlayerState` access where possible.
- [x] Feed camera render data from `PlayerState`.
- [x] Feed HUD health/stamina from `PlayerState`.
- [ ] Feed player noise pulses from controller output instead of renderer side effects.
- [ ] Preserve flashlight visual presentation in renderer until render/gameplay split is ready.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] WASD movement feels unchanged.
- [ ] Mouse look feels unchanged.
- [ ] Sprint stamina drain/regeneration works.
- [ ] Crouch height/speed works.
- [ ] Jump/gravity works.
- [ ] Exit interaction still works.
- [ ] Screensaver autopilot behavior still works.

Completion notes:

- Agent: Codex
- Date: 2026-06-02
- Player fields moved: camera position, yaw, pitch, body yaw, health, stamina, stamina regen delay, sprint stamina lock, vertical offset, crouch/tunnel posture, interaction/flashlight latch fields, visited tiles, audible noise radius, step phase, smoothed move speed, run intensity, run effort, and breath phase now live on `PlayerState`.
- Player fields intentionally left in renderer: manual look deltas, autopilot/path-following camera behavior state, and render-only camera effects remain renderer fields until the controller boundary is defined.
- Behavior changes: none intended. Release build and `/selftest` passed after the `PlayerState` data model migration.
- Controller shell: added `PlayerController::UpdateStamina` as the first controller-owned behavior slice. It updates `PlayerState` stamina/regen/lock fields and returns sprint/jog/target-speed decisions while collision, camera, interaction, and autopilot behavior remain in the existing renderer path.
- Crouch slice: added `PlayerController::UpdateCrouch` for crouch blend plus derived speed, eye-height, eye-response, and bob scale values. Tunnel detection, collision recovery, camera placement, interactions, and autopilot remain in the existing renderer path for later slices.
- Look slice: added `PlayerController::UpdateManualLook` for mouse sensitivity, invert-Y handling, look smoothing, yaw update, and pitch limits.
- Movement intent slice: added `PlayerController::BuildMoveIntent` and `PlayerController::UpdateManualMove` for input normalization, desired move direction, speed acceleration/deceleration, and move distance.
- Manual horizontal move slice: added `PlayerController::UpdateManualHorizontalMove` so manual move speed/state updates and collision move requests are controller-owned. The renderer still provides the `MoveCameraSafely` collision primitive through a callback until camera-state migration gives the controller a cleaner world/collision interface.
- Controller implementation shell: split inline `PlayerController` method bodies into `src/game/player_controller.inl`, leaving `player_controller.h` as the controller input/output declarations plus the temporary implementation include.
- Collision request slice: added `PlayerCollisionMoveRequest` so controller movement code sends a named collision request to the host instead of loose movement arguments. The renderer still implements the actual maze/camera collision response.
- Collision service slice: moved tile passability, player footprint checks, segment traversal checks, footprint recovery, and safe movement into `src/game/player_collision_service.inl`. The service is still renderer-backed because it needs tunnel posture, monster-body occupancy, path recovery, and step-phase side effects, but manual and autopilot movement now go through the explicit `PlayerCollision*`/`MovePlayerThroughCollision` surface instead of camera-named helpers.
- Run motion slice: added `PlayerController::UpdateRunMotion` for manual move/run blend, run intensity, run effort, and breath phase. Renderer still owns bob composition, camera placement, and autopilot run-motion behavior.
- Step phase slice: added `PlayerController::AdvanceStepPhase` for stride/run-blend step-phase advancement. Renderer collision/transition paths now call the controller directly through an explicit `PlayerMovementTuning` input.
- Manual camera height slice: added `PlayerController::UpdateManualCameraHeight` for manual head bob, side bob, breath offset, crouch eye height, and smoothed camera Y composition. Renderer still supplies live settings and reveal-map side effects around the controller call.
- Input latch slice: added `PlayerController::UpdateFlashlightInput`, `ConsumeInteractPress`, and `ResetInputLatches`. Manual play, score-screen input, flashlight toggles, and lifecycle latch reset now use the controller directly. Saved-run restore still writes restored latch fields through `GameWorld` because that path is state restoration rather than input handling.
- Interaction helper slice: moved collectible pickup and save-point interaction helpers into `src/game/player_interactions.inl`, leaving manual movement to consume controller interaction intent and then route the intent to the appropriate game-world/save behavior.
- Manual update shell: added `PlayerManualControlInput`, `PlayerManualControlResult`, and `PlayerController::UpdateManualControl`. The method now owns manual look, move intent, crouch blend, stamina, horizontal movement request, interaction latch, run motion, step phase, and manual camera height sequencing. Renderer callbacks still supply tunnel posture/lean updates, footprint recovery, and maze collision movement until those services can move behind explicit world/collision contracts.
- Manual service contract: replaced anonymous `UpdateManualControl` callback parameters with a named `PlayerManualControlServices` contract for posture update, footprint recovery, and collision movement. The renderer bridge now wires those service functions explicitly, making the remaining renderer dependencies visible at the controller boundary.
- Manual bridge adapter slice: `src/game/player_manual_controller_bridge.inl` now builds controller input, renderer-backed manual services, and manual interaction dispatch through named helper functions instead of embedding the remaining renderer service adapters inside `UpdateManualPlayer`.
- Jump/gravity audit: `PlayerState` already has dormant vertical velocity, grounded, and jump-request fields, but current code has no active jump/gravity behavior beyond reset/snapshot state. There is no live jump/gravity logic to move in this slice.
- Motion state slice: moved step phase, smoothed move speed, run intensity, run effort, and breath phase from renderer state to `PlayerState`; audio, render, snapshot, manual control, and autopilot now read/write those fields through `playerState_`.
- Camera state slice: moved the active camera position, yaw, body yaw, and pitch storage from renderer fields to `PlayerState`; rendering, audio, saves, menus, manual play, and autopilot now read/write the active view through `playerState_`.
- Player reset slice: added `PlayerState::ResetSessionState` so renderer reset code no longer hand-assigns each gameplay-owned player field.
- Player restore/snapshot slice: added `PlayerState::RestoreSavedRunState` plus `PlayerSnapshotState` capture/restore so saved-run and pause/resume paths no longer restore player-owned fields one assignment at a time in the renderer.
- Screensaver/controller separation: `renderer_update.inl` dispatches `UpdateManualPlayer` only for `GameSessionInputSource::Manual`; screensaver sessions are launched with `GameSessionInputSource::Autopilot`, so the manual `PlayerController` path is not used by the screensaver host.
- Player/camera helper split: `src/game/player_camera_movement.inl` is now an include coordinator. Temporary renderer-private `.inl` slices own debug cameras, session reset/restart, main menu scene camera simulation, monster spatial helpers, map visibility, runtime random/effect tuning, monster pressure/dread/blood presentation, camera pose/flashlight aim, air particles, navigation/collision, camera attention, path scoring, session transitions, autopilot path selection, manual controller bridging, and autopilot path following. This is a mechanical extraction; the collision service and presentation-heavy effects remain renderer-backed until those boundaries can become real world/render/audio units.
- Monster pressure split: `src/game/player_monster_pressure.inl` is now a coordinator over monster focus/preview camera helpers, dread/chase pressure helpers, and blood reveal/proximity pressure helpers.
- Runtime/session type slice: moved `RendererRuntimeMode` and `IsPlayableSimulationMode` out of the old `src/render/render_types.inl` into `src/game/runtime_mode.h`, and moved `CollectiblePage`/`SavePoint` into `src/game/game_world_types.h`. The remaining render structs now live in `src/render/render_types.h`.
- Renderer include-boundary slice: `Settings`, playable progression types, `GameSessionRuntimeState`, and `GameWorld` are now included from `src/main.cpp` before the renderer private include block. They are no longer created as renderer-private anonymous-namespace types, which clears the next prerequisite for normal game-world translation units.
- Verification: Release build passed for `BackroomsMaze.scr` and `BackroomsMazeGame.exe`; `BackroomsMazeTest.exe /selftest` returned exit code 0.

## Phase 4: Extract Monster AI State

Goal: separate monster decisions from monster rendering.

### 4.1 Monster Data Model

- [x] Create `src/monster/monster_state.h`.
- [x] Create `src/monster/monster_state.cpp`.
- [x] Add monster position and yaw fields.
- [x] Add path, path index, and goal tile fields.
- [x] Add sound target and last-known target fields.
- [x] Add roam target field.
- [x] Add search/chase timers and transition flags.
- [x] Add recognition and curiosity timers.
- [x] Add sensory flags for seen/heard player.
- [x] Decide whether body/limb simulation is gameplay-owned or render-owned.

### 4.2 Monster AI API

- [x] Create `src/monster/monster_ai.h`.
- [x] Create `src/monster/monster_ai.cpp` or temporary `.inl` shell if needed.
- [x] Define update inputs: `Maze`/navigation, player state, sound pulses, settings, `dt`.
- [x] Define initial update outputs: visibility/hearing/goal/movement summary.
- [x] Add explicit death/contact event output.
- [x] Add explicit scare/gameplay event outputs.
- [x] Move target selection logic.
- [x] Route player sound pulses through the monster AI input contract.
- [x] Move settings-only monster sight distance calculation into monster AI API.
- [x] Move sight/hearing checks.
- [x] Move search/chase/roam transitions.
- [x] Move monster path refresh logic.
- [x] Remove monster eye lights, eye shadow setup, eye dynamic geometry, and eye buffers.

### 4.3 Renderer Integration

- [x] Read monster position from `MonsterState`.
- [x] Read monster sensory flags from `MonsterState`.
- [x] Read monster path/debug data from `MonsterState`.
- [x] Keep monster mesh/skull loading in renderer.
- [x] Keep monster shadow resources in renderer.
- [x] Keep monster dynamic vertex generation in renderer until a visual-animation boundary exists.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Monster can still roam/search/chase.
- [ ] Monster can still react to player sound.
- [ ] Debug AI minimap still shows monster goal/path/sound target.
- [ ] Monster visual rendering remains intact.

Completion notes:

- Agent: Codex
- Date: 2026-06-02
- Monster fields moved: position, yaw, path, path index, goal tile, sound target tile, last-known target tile, roam target tile, has-sound flag, has-last-known flag, repath timer, chasing-visible flag, heard-player-now flag, can-see-player-now flag, kill count, search timer, roam timers, recognition state/timers, and curiosity timers now live on `MonsterState`.
- Monster fields intentionally left in renderer: render trails, limb anchors, smoothed body points/up vectors, body smoothing timestamps, render visibility, handprints, head bob/scan offsets, and render-only monster presentation fields.
- Monster AI API slice: added `monster_ai.h` with `MonsterUpdateInput` and `MonsterUpdateOutput`; the existing renderer-included `monster_ai.inl` now exposes `BuildMonsterUpdateInput` and updates through `UpdateMonster(const MonsterUpdateInput&)`.
- Monster event-output slice: `MonsterUpdateOutput` now carries `killPlayer` and `distanceToPlayer`; the renderer update loop consumes that result instead of rebuilding the kill-distance/line-clear condition inline.
- Monster hearing-input slice: `MonsterUpdateInput` now carries the mutable player sound pulse list, and monster hearing processing consumes that input instead of directly reading the renderer member.
- Monster sight/hearing slice: moved the settings/fog-limited sight distance calculation to `MonsterSightDistanceMeters` in `monster_ai.h`; monster sight now uses monster/player/maze line-of-sight without renderer-generated eye anchors, and hearing consumes sound pulses through `MonsterUpdateInput`.
- Monster path-refresh slice: extracted path refresh/repath/empty-path reset behavior into `RefreshMonsterPathToGoal`, reducing the monolithic monster update step while keeping the same maze pathing behavior.
- Monster target-selection slice: extracted chase, last-known search, sound-target pursuit, and passive roam target transitions into `UpdateMonsterTargetSelection`.
- Monster reset slice: added `MonsterState::ResetSessionState` so renderer reset code no longer hand-assigns every gameplay-owned monster field.
- Monster snapshot slice: added `MonsterSnapshotState` capture/restore so pause/resume snapshots now group monster AI state separately from renderer-owned monster presentation fields.
- Monster eye removal: deleted monster eye calibration settings/UI, eye dynamic geometry, eye-light shadow resources, eye-light render pass, shader eye-light contribution, and render-produced eye anchors. Monster focus reactions now use `MonsterFocusPoint`.
- Monster chase-pressure slice: moved chase memory and chase panic into `MonsterState`, added named recognition/curiosity/chase-pressure helpers, and removed the duplicate renderer pause-snapshot fields for that state.
- Monster runtime split: `src/monster/monster_ai.inl` is now a coordinator over renderer-private monster senses/awareness, runtime presentation, collision/movement, path/target selection, hearing/sound alert, debug walk, and update-coordinator slices. This is still renderer-included code, but the remaining monster dependencies are now grouped by responsibility.
- Monster gameplay event slice: `MonsterUpdateOutput` now reports typed gameplay events such as `KillPlayer` instead of exposing the monster contact death condition as a loose renderer-facing boolean. The renderer still consumes the event and performs the existing death presentation.
- Behavior changes: monster eye visuals/eye lights are intentionally removed, and monster sight no longer depends on eye-cone render anchors. Release build passed after the `MonsterState` data model migrations.

## Phase 5: Extract Game World

Goal: centralize gameplay simulation outside renderer.

### 5.1 Game World Shell

- [x] Create `src/game/game_world.h`.
- [x] Create `src/game/game_world.cpp` or temporary `.inl` shell if needed.
- [x] Add `GameWorld` ownership of `Maze`.
- [x] Add `GameWorld` ownership of `PlayerState`.
- [x] Add `GameWorld` ownership of `MonsterState`.
- [x] Add `GameWorld` ownership of collectibles/page state.
- [x] Add `GameWorld` ownership of death and exit transition state.
- [x] Add `GameWorld` ownership of gameplay sound pulses/events.

### 5.2 Simulation Update

- [ ] Define `GameWorld::Initialize` inputs currently needed by renderer startup.
- [ ] Define `GameWorld::Update` inputs: input snapshot, settings, `dt`.
- [ ] Move playable-game simulation dispatch out of `renderer_update.inl`.
- [ ] Move death/exit progression that is gameplay-owned.
- [ ] Move collectible/page pickup progression.
- [ ] Move gameplay scare event triggering if it affects game state.
- [ ] Keep visual-only particle updates in renderer until an effects boundary exists.

### 5.3 Render Snapshot

- [x] Define `GameWorldSnapshot`.
- [x] Include camera/player render state.
- [x] Include monster render state.
- [x] Include maze render references or stable IDs.
- [x] Include HUD data.
- [x] Include debug overlay data.
- [ ] Update renderer to consume snapshot for playable game rendering.

### 5.4 Debug Integration

- [ ] Expose debug read-only world state access.
- [ ] Expose debug override hooks only where currently needed.
- [ ] Keep effect slice viewer independent from playable `GameWorld` unless intentionally merged.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Main menu still works.
- [ ] Single Player starts from a clean world.
- [ ] Returning to menu and starting again resets world state correctly.
- [ ] Debug still opens and returns.

Completion notes:

- Agent: Codex
- Date: 2026-06-02
- World-owned systems: `GameWorld` now owns `Maze`, `PlayerState`, `MonsterState`, `PlayableRunState`, collectible pages/count, save point state, player audible sound pulses, and the death/exit active flags/timers. `GameWorld::ResetSessionState` resets the first session-level transient state used by renderer reset paths.
- Progression slice: `PlayableRunState::AdvanceRunningTimers` now owns the run/level timer increment once the renderer has gated that progression is allowed.
- Collectible pickup slice: `GameWorld::CollectPage` now owns page collected state, session page count, and playable layer page counters; the renderer still owns raycast detection, audio, and HUD notification.
- Collectible generation slice: `GameWorld::ResetCollectiblePagesForGeneration` and `IsLayerPageCollected` now own collectible reset and layer-collected checks; renderer still owns page placement geometry.
- Save point slice: `GameWorld` now owns save point spawn eligibility, budget, candidate tile selection, chance roll, yaw choice, activation state, and spawn budget increment; the renderer still supplies the playable-mode gate, configured spawn chance, RNG, and interaction audio/HUD.
- Save point interaction slice: `GameWorld::CanInteractWithSavePoint` now owns the world/player/maze eligibility query; the renderer still owns save-file writes and notifications.
- Death/exit transition slice: `GameWorld` now owns beginning and advancing death/exit transitions, including active flags, timers, and monster kill count; the renderer still owns visual/camera cleanup and save-file side effects.
- Flashlight input slice: `GameWorld::UpdateFlashlightInput` now owns the flashlight press latch and enabled toggle; the renderer still owns click audio and audible-noise emission.
- Interact input slice: `GameWorld::ConsumeInteractPress` now owns the interact edge/latch used by score screens and manual gameplay; the renderer still owns the resulting commands.
- Input reset slice: `GameWorld::ResetInputLatches`, `RestoreFlashlightState`, and `RestoreInteractLatch` now own session/restore input latch setup.
- Playable state predicate slice: `GameWorld` now owns playable run finished, score-screen, level-running, boss-level-running, progression-enabled storage, and progression-advance predicates; the renderer still supplies host runtime mode policy.
- Playable page distribution slice: layer page distribution, per-level page counts, per-level collected counts, and page-disabling helpers now live on `PlayableRunState`.
- Playable completion slice: `PlayableRunState::CompleteCurrentLevel` now owns level result bookkeeping, total score, completed-level list, score-screen flags, layer secret counts, and final-run flags; renderer still owns save deletion, settings changes, path cleanup, and notifications.
- Score-screen continuation slice: `PlayableRunState` now owns the continue eligibility check and next-level selection after a score screen.
- Playable run setup slice: `PlayableRunState` now owns reset, normal run setup, custom run setup, and per-level start state; renderer still supplies level specs, RNG choices, settings application, maze restart, and notifications.
- Playable level spec slice: `PlayableRunState::BuildLayerOneLevelSpec` now owns layer-one level dimensions, scare tiers, boss encounter probabilities, and RNG ordering for normal playable runs; renderer still applies settings and starts the generated maze.
- Playable level settings slice: `PlayableLevelSpec::ApplyRuntimeSettings` now owns playable level settings policy, including maze dimensions, level-scaled clutter/paper density, dark-layer-one lighting, scare-tier toggles, and boss movement/visibility settings. Renderer still supplies the preserved lamp-flicker RNG roll and clears monster presentation when the level has no boss.
- Custom scare delay slice: `PlayableRunState::BeginCustomRun` now owns custom scare start-delay rolls, preserving the existing custom-run RNG order.
- Custom game spec slice: `CustomGameSpec::Normalize` now owns custom spec bounds and odd-dimension normalization before custom run setup.
- Custom level spec slice: `CustomGameSpec::ScareTier` and `CustomPlayableLevelSpec` now own custom scare-tier and playable level spec derivation.
- Custom scare query slice: `CustomGameSpec::AnyScareEnabled` and `MaxScareChancePercent` now own scare-toggle aggregate queries used by settings application.
- Custom runtime settings slice: `CustomGameSpec::ApplyRuntimeSettings`, `ApplyEnvironmentSettings`, and `ApplyScareSettings` now own custom-game environment/scare settings application; renderer still applies benchmark overrides and rebuilds the maze.
- Custom start notice slice: `CustomGameSpec::WriteStartNotice` now owns custom-run start notice text derived from the custom spec.
- Saved-run restore slice: `PlayableRunState` now owns saved playable-run reconstruction, save-item counter clamping, layer-page marking/reconciliation, custom scare-delay restore, and current-level restore; renderer still owns save-file parsing and resource/maze rebuild order. `GameWorld` owns save-point and collectible restore helpers used after saved maze geometry is rebuilt.
- Saved-run write slice: `PlayableRunState::WriteSavedRunFields` now owns serialization of playable-run fields, custom spec fields, custom scare delays, save-item counters, page distribution, score/timer values, and current-level spec values. Renderer persistence still writes the file version, maze bytes, player position/vitals, collectibles, and save point state.
- Saved-run parse slice: shared saved-run key/value parsing helpers plus `PlayableRunState::ReadSavedRunFields` now own playable-run save-file parsing. Renderer no longer builds the playable restore DTO field by field.
- Saved-world persistence slice: `GameWorld::WriteSavedRunFields`, `ReadSavedMazeRestoreState`, `RestoreSavedMazeGeometry`, `ReadSavedRuntimeRestoreState`, and `RestoreSavedRuntimeState` now own maze/player/save-point/collectible save fields. Renderer still owns file IO, resource rebuild order, and session-mode flags after load.
- GameWorld translation-unit slice: `src/game/game_world.cpp` now owns reset/snapshot/layout helpers, saved-run maze/runtime serialization helpers, player latch restore helpers, playable-run facade/scoring/progression helpers, save-point/death/exit helpers, collectible generation/targeting/mutation/restore helpers, and player sound/audio event queue helpers. `src/game/game_world.h` now exposes declarations for behavior instead of inline method bodies.
- Pause snapshot slice: added `GameWorldSnapshotState` capture plus split generation/runtime restore so pause/resume snapshots group world-owned maze, player, monster, progression, collectible, save-point, sound-pulse, and transition state without changing the mesh rebuild order.
- Render snapshot slice: added `GameWorldRenderSnapshot` plus `GameWorld::BuildRenderSnapshot` as the render-facing world view. It exposes player camera/HUD data, monster render/debug inputs, collectible/save-point state, transition flags, and a stable maze reference without changing existing renderer call sites yet.
- Render snapshot consumer slice: collectible/save-point dynamic geometry, dynamic effect culling, HUD health/stamina bars, minimap/AI-debug overlay maze/monster/sound-pulse reads, present-camera setup, scene constant population, monster presentation geometry, menu scene projection/placement, runtime texture creation/update, visibility/shadow culling, static mesh chunking/finalization helpers, and maze/lamp placement helpers now consume `GameWorldRenderSnapshot` instead of directly reaching into `gameWorld_`.
- Renderer world-access slice: render-side direct `player`, `monster`, death-timer, and exit-transition field reads have been removed from renderer includes. Maze generation dispatch now goes through `GameWorldMazeGenerationRequest`/`GameWorld::GenerateMaze`; remaining direct world access is in game-side session reset/simulation includes and audio/scare follow-up areas.
- Collectible placement write slice: `GameWorld::PlaceCollectiblePage` now owns page-slot mutation during renderer-driven placement.
- Collectible generation slice: `GameWorld::GenerateCollectiblePagesForCurrentLevel` now owns collectible page placement, including layer page ranges, collected-page skipping, tile selection, wall/floor placement, and page orientation. Renderer still supplies the playable-mode gate, wall height, and RNG.
- Collectible targeting slice: `GameWorld::FindCollectiblePageInView` now owns the player/maze/page raycast used before collection; renderer still owns collection audio and HUD notification.
- Score calculation slice: `GameWorld::ScoreCompletedPlayableLevel` now owns playable level score calculation from run state, player health, and collected pages.
- Save point generation slice: `GameWorld::TryGenerateSavePoint` now owns playable save-point generation over world state, including candidate choice, forced-spawn logic, chance roll, yaw choice, and budget mutation; renderer still owns mesh rebuild and save-file side effects.
- Save point budget slice: `PlayableRunState::BuildSavePointSpawnPlan` now owns save-point level eligibility, remaining save-item budget, and forced-spawn calculation; `GameWorld` consumes that plan while choosing world placement.
- Maze settings slice: `GameWorld::ApplyMazeSettings` now owns settings-to-maze dimension/tile/exit assignment used by startup, menu/debug setup, playable level start, custom run start, and maze restart paths.
- Save item target slice: `PlayableRunState::PickSaveItemTarget` now owns the per-run save-item target choice during normal and custom run setup.
- Player sound pulse slice: `GameWorld` now owns player sound pulse lifetime, capping, and noise-radius recomputation; renderer audio still decides when to emit pulses.
- World alias cleanup slice: removed renderer aliases for player sound pulses, progression-enabled state, collectible pages/count, save point, playable run state, and death/exit transition flags/timers; remaining consumers now route through `gameWorld_` directly.
- Monster cleanup slice: `MonsterState::ClearPursuitState` and `GameWorld::ResetMonsterKillCount` now own renderer/progression-facing monster pursuit cleanup and kill-count reset. Renderer presentation cleanup is grouped behind `ResetMonsterPresentationState` and `PrimeMonsterTrail` while monster rendering remains renderer-owned.
- Player vitals slice: `PlayerState` now owns kill, stamina refill, full-vitals restore, and saved-vitals clamping; `GameWorld::BeginPlayerDeath` owns the health-zero mutation paired with death start while renderer still owns death visuals and save deletion.
- Custom scare gate slice: `PlayableRunState::CustomScareGateFor` now owns custom scare delay/chance eligibility; renderer scare effects still perform the actual random roll to preserve RNG consumption behavior.
- Playable presentation query slice: `PlayableRunState` now owns map dirt progression, air-particle density scale, dark-layer lighting applicability, current-level boss queries, score/time readouts, and level-start notice text used by renderer presentation.
- Score calculation slice: `GameWorld::ScoreCurrentPlayableLevel` now owns current-level score input selection from world/run state; renderer progression still decides when completion happens and formats notifications.
- Playable session facade slice: save/load, playable level start, custom/normal run setup, score-screen continuation, completion scoring, and timer advancement now call through `GameWorld` instead of the renderer mutating `PlayableRunState` directly.
- Effect runtime grouping slice: `EnvironmentalEffectRuntimeState` now groups lamp runtime data, spark particles/chains/flashes, steam emitters/particles, vent drops, wet tile/drip/footprint state, air particles, air particle budget state, and lamp-damage texture state. `ScareEffectRuntimeState` now groups blood scare points/reveal regions, blood/flesh/vision flicker timers, focus reaction state, proximity scare cooldowns, and scare tile tracking. Pause snapshots store these grouped records directly.
- Scare/effect helper split: `src/gameplay/scare_effect_events.inl` is now a coordinator over sensory queries, lamp/spark damage events, vent scare events, scare trigger updates, and spark/steam/drop particle simulation. Scare execution remains renderer-backed because it still drives presentation particles, audio queues, and view reactions.
- Camera/autopilot runtime grouping slice: `PlayerCameraRuntimeState` now groups player camera navigation/attention state used by autopilot-style movement, including paths, path index, head scans, look-backs, branch/room surveys, turn-look blending, manual look deltas, prop look points, and threat repath timing. Pause snapshots store the grouped record directly.
- View runtime grouping slice: `PlayerViewRuntimeState` now groups camera presentation behavior that is separate from path navigation, including prop glances, exit focus, danger/dread meter values, chase look-backs, stumble reactions, flashlight aiming/agitation/darts/snaps/holds, vent reactions, fade-in, and exit-walk camera start state. Pause snapshots store this grouped record directly.
- Monster presentation grouping slice: `MonsterPresentationRuntimeState` now owns render-only monster trail, limb anchors, body smoothing cache, handprints, render visibility hold, and head animation offsets/blends. Pause snapshots use `MonsterPresentationSnapshotState` plus capture/restore helpers to preserve the old snapshot surface without making body-smoothing cache persistent.
- Session runtime slice: `GameSessionRuntimeState` now owns runtime mode, input source, minimap style, live input, gameplay settings, session seed, and renderer RNG. Shared launch/restore paths use named configure helpers for playable-manual and screensaver-autopilot sessions.
- Menu/runtime presentation slice: `MainMenuRuntimeState`, `MapOverlayRuntimeState`, `HudNotificationRuntimeState`, `MonsterPreviewRuntimeState`, `BenchmarkRuntimeState`, `PresentRuntimeState`, `StartupProgressRuntimeState`, and `GpuProfileRuntimeState` now group formerly loose renderer runtime flags/caches.
- Render asset/geometry slice: `RenderAssetRuntimeState`, `StaticSceneGeometryRuntimeState`, and `DynamicGeometryRuntimeState` now group loaded meshes, static index/chunk/instancing counts, and per-frame dynamic vertex output.
- Door/light presentation slice: `ExitDoorPresentationState` and `FixtureShadowRuntimeState` now own exit-door presentation geometry/light/open-angle state and current fixture shadow inputs.
- Renderer host/device slice: `RendererHostRuntimeState`, `RendererTimeRuntimeState`, `RendererDebugRuntimeState`, `RendererDeviceRuntimeState`, `RenderTargetRuntimeState`, and `ShadowResourceRuntimeState` now group HWND/size, frame clocks, debug timers, D3D device/swap-chain, render-target/back-buffer, and shadow-map resources.
- Renderer pipeline/resource slice: `ShaderRuntimeState`, `InputLayoutRuntimeState`, `PipelineStateRuntimeState`, `RenderBufferRuntimeState`, `MaterialTextureRuntimeState`, and `RuntimeTextureResourceState` now group shader programs/layouts/states, GPU buffers, material textures, and runtime/generated textures.
- Session/public-type slice: `GameSessionSpec`, `GameSessionRuntimeState`, session input source, and minimap style now live in `src/game/game_session.h`; custom game menu control IDs live in `src/game/custom_game_menu.h`; the old renderer public catch-all was split into focused monster-preview and menu-presentation headers.
- World alias cleanup slice: removed renderer compatibility aliases for `Maze`, `PlayerState`, and `MonsterState`; renderer-included modules now access those objects explicitly through `gameWorld_`.
- Renderer settings slice: `RendererSettingsRuntimeState` now owns the renderer live settings copy, making the separate session gameplay settings and live render settings explicit.
- Renderer-owned systems remaining: scare effect execution, low-level audio scheduling/playback, monster presentation rendering, and grouped D3D/render resources remain renderer-owned.
- Behavior changes: none intended for the initial world ownership shell.

## Phase 6: Extract Audio Event Flow

Goal: stop gameplay audio from being renderer-owned timing logic.

Future audio note:

- Realistic audio propagation is intentionally out of scope for this structural pass. The target is eventually sound propagation through connected hallways, with thick walls blocking sound, likely backed by a fast GPU/compute propagation path. The event-flow refactor should preserve enough event data to feed that system later instead of baking in the current direct wall-count heuristics.

### 6.1 Audio Event Types

- [x] Create `src/audio/game_audio_events.h`.
- [x] Define event type for footsteps.
- [x] Define event type for door open/close/lock.
- [x] Define event type for lamp buzz/flicker/break.
- [x] Define event type for vent puffs/groans.
- [x] Define event type for monster vocals.
- [x] Define event type for wet drips/water effects.
- [x] Define event type for scare/effect stingers.
- [ ] Include position, volume, pitch, bus, spatialization, and occlusion hints as needed.

### 6.2 Game Audio System

- [x] Create `src/audio/game_audio_system.h`.
- [x] Create `src/audio/game_audio_system.cpp` or temporary `.inl` shell if needed.
- [x] Move `GameAudioEvent` factory methods into `src/audio/game_audio_events.cpp`.
- [x] Move delayed audio event scheduling out of renderer.
- [ ] Move footstep audio triggering out of renderer.
- [ ] Move monster vocal timing out of renderer if gameplay-owned.
- [ ] Move door audio transition logic out of renderer if gameplay-owned.
- [ ] Keep low-level `AudioEngine` sample loading and playback separate.

### 6.3 Monster Hearing Integration

- [ ] Make player movement emit sound/noise events.
- [x] Make loud world effects emit sound/noise events.
- [ ] Feed monster hearing from sound/noise events instead of renderer-owned pulse fields.
- [ ] Keep audio occlusion queries routed through maze/navigation APIs.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
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

Audio event notes:

- Agent: Codex
- Date: 2026-06-02
- Audio logic moved: added `GameAudioEvent` and a `GameWorld` event queue drained by renderer audio in the same frame. Page pickup, flashlight click/noise, footstep playback/noise, lamp spark/break/click, vent puff, wet drip, door, and scare stinger/noise events now travel through typed audio events or through the shared event dispatch path. `PlayerAudibleSoundPulse` moved out of renderer/render types into the audio event boundary. `DelayedAudioEvent`, `LampHumCandidate`, and `AudioRuntimeState` moved into `src/audio/game_audio_system.h`, with audio timers/latches/caches and scene/menu-door reset helpers grouped under `AudioRuntimeState`.
- Audio owner state slice: `RendererAudioRuntimeState` now groups the renderer-owned `AudioEngine`, readiness/sample-loaded flags, and gameplay audio runtime timers. This preserves current playback behavior while keeping the future hallway-propagation audio redesign out of the renderer field list.
- Audio queue slice: `AudioRuntimeState` now owns delayed event capacity, event construction, countdown, and draining. Delayed monster vocal echoes carry `GameAudioEventCategory::MonsterVocal` metadata for future propagation. Renderer still supplies the ready-event playback callback so XAudio playback and occlusion remain unchanged.
- Footstep trigger slice: `AudioRuntimeState::ConsumeFootstepTrigger` now owns step-phase trigger detection. Renderer still owns wet-footstep detection, volume/hearing-radius calculation, and dispatching the resulting footstep `GameAudioEvent`.
- Renderer audio split: `src/audio/renderer_audio.inl` is now a coordinator over wet floor/ceiling audio surfaces, lamp hum/flicker audio, game audio event dispatch and hearing radii, monster vocal/vent-groan audio, and renderer audio spatial/update coordination. The split preserves current low-level XAudio and occlusion behavior while making the upcoming hallway-propagation audio rewrite boundary explicit.
- Audio logic left in renderer: low-level XAudio playback, sample choice, layered/direct monster vocal playback, direct occlusion heuristics, persistent lamp hum emitters, footstep sound construction, monster hearing pulses, and realistic propagation remain renderer/audio-owned for the upcoming dedicated audio rework.
- Behavior changes: none intended.

## Phase 7: Split Static Maze Rendering

Goal: reduce `renderer_maze_mesh.inl` into smaller render-building modules.

### 7.1 Preparation

- [x] Add section comments to `renderer_maze_mesh.inl` marking geometry primitives.
- [x] Mark wall/floor/ceiling mesh generation.
- [x] Mark prop placement.
- [x] Mark lamp placement.
- [x] Mark decals/damage/effects placement.
- [x] Mark exit geometry.
- [x] Mark final buffer/index upload logic.
- [x] Identify shared helper functions and local structs.

### 7.2 File Splits

- [x] Create `src/render/renderer_geometry_primitives.inl` or `.cpp` when dependencies allow.
- [x] Create `src/render/renderer_wall_floor_ceiling_mesh.inl` or `.cpp`.
- [x] Create `src/render/renderer_prop_placement.inl` or `.cpp`.
- [x] Create `src/render/renderer_lamp_placement.inl` or `.cpp`.
- [x] Create liquid/decal/damage placement helper module.
- [x] Create `src/render/renderer_exit_geometry.inl` or `.cpp`.
- [x] Create `src/render/renderer_static_mesh_helpers.inl` or `.cpp`.
- [x] Create prop mesh sizing/selection helper support.
- [x] Create instanced static prop support helpers.
- [x] Create baked prop shadow placement helper support.
- [x] Create shared maze placement helper support for prop/liquid/decal fitting.
- [x] Create shared liquid/water/decal type and side-helper support.
- [x] Create static scene finalization/upload helper support.
- [x] Update `renderer.inl` include order or CMake sources accordingly.

### 7.3 Determinism Checks

- [ ] Confirm maze seed produces the same base layout.
- [ ] Confirm prop placement is not unintentionally changed.
- [ ] Confirm lamp placement is not unintentionally changed.
- [ ] Confirm exit geometry is not unintentionally changed.
- [ ] Confirm water/blood/damage decal placement is not unintentionally changed.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
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

Static maze rendering notes:

- Agent: Codex
- Date: 2026-06-02
- Files split: `src/render/renderer_geometry_primitives.inl` now owns reusable quad/vector/card/box primitive helpers. `src/render/renderer_static_mesh_helpers.inl` now owns static prop append, material UV helpers, and static index chunking. `src/render/renderer_exit_geometry.inl` now owns exit portal selection. `src/render/renderer_maze_mesh_setup.inl` now owns mesh-build runtime reset bookkeeping. `src/render/renderer_wall_floor_ceiling_mesh.inl` now owns deterministic floor, ceiling, wall-run, wall-feature interior surface helpers, exit doorway/vestibule geometry, and exit-aware wall-run routing. `src/render/renderer_maze_placement_helpers.inl` now owns reusable footprint/reservation/fitting helpers used by props, papers, and liquid/decal placement plus open placement tile collection. `src/render/renderer_prop_mesh_helpers.inl` now owns prop mesh span lookup, cabinet dimensions, and chair mesh selection. `src/render/renderer_prop_placement.inl` now owns the wall vent, metal cabinet, furniture, cassette, loose-paper, room-clutter, debug prop inspection, exit door/sign placement slices, and ambient/scatter prop placement batches. `src/render/renderer_liquid_placement.inl` now owns shared liquid/water/decal structs, material helpers, side helpers, liquid damage coverage bookkeeping, center-seep coverage helpers, ceiling footprint reservations, liquid floor/ceiling canvas marking/emission, water wall canvas runs, pending floor-seam queue/flush helpers, shared wall-leak frame construction, liquid footprint scale/projection fit helpers, water floor border continuation, tile-edge touch calculation, blood wall liquid card emission, and generic liquid scare-point creation. `src/render/renderer_water_surface_placement.inl` now owns water tile marking, water surface damage batching, water blob marking, floor/wall water-pool card emission and merging, water tile/bridge/card emission, and water wall-card/runoff/spill placement. `src/render/renderer_blood_damage_placement.inl` now owns generic liquid floor/ceiling overlay placement, blood center/burst/leak placement, blood placement batching, and blood ceiling propagation. `src/render/renderer_water_like_damage_placement.inl` now owns water-like fit policies, center/leak placement, and water-like damage emission over the shared liquid canvas helpers. `src/render/renderer_instanced_static_props.inl` now owns instanced static mesh registration, transform bounds, pending instance construction, and grounded prop instancing support. `src/render/renderer_prop_shadow_placement.inl` now owns baked prop shadow lamp-grid projection and shadow card creation. `src/render/renderer_lamp_placement.inl` now owns ceiling lamp mesh/effect placement and lamp damage texture updates. `src/render/renderer_static_scene_finalize.inl` now owns floor/ceiling final append, static index chunking, instanced static chunk building, startup geometry profiling, and static/dynamic buffer creation. `renderer_maze_mesh.inl` now primarily owns mesh build orchestration and shared context construction.
- Dynamic geometry split: `src/render/renderer_dynamic_geometry.inl` is now a coordinator over dynamic primitive helpers, skull mesh transforms, exit-door/menu dynamic geometry, monster dynamic geometry, particle/page/save-point dynamic geometry, and dynamic buffer/constant/lamp-damage uploads. Monster body geometry remains one large renderer-owned function for now because it has dense local helper state and presentation-specific dependencies.
- Overlay split: `src/render/renderer_overlays.inl` is now a coordinator over shared overlay draw submission, HUD notification texture/draw helpers, postprocess draw, playable HUD/dread meter, and minimap/AI debug map drawing.
- Present split: `src/render/renderer_present.inl` is now a coordinator over render preflight/camera setup, scene constant-buffer population, dynamic geometry update, visibility/chunk draw helpers, flashlight/fixture shadow passes, main scene passes, and postprocess/overlay/present cleanup.
- Present constants split: scene constant-buffer setup is further split into lighting/material/transition constants, horror/blood constants, and air/exit/monster-fog constants.
- Determinism notes: helper extraction only; no RNG, placement, material, or geometry math changes intended. Unreachable manual fallback geometry for chairs, tables, tipped chairs, and metal cabinets was removed after those paths had already returned from the instanced prop path.
- Behavior changes: none intended. Release build passed for `BackroomsMaze.scr` and `BackroomsMazeGame.exe`; `BackroomsMazeTest.exe /selftest` returned exit code 0 after the liquid canvas/seam helper extraction.

## Phase 8: Split Shaders

Goal: make shader work manageable without searching one multi-thousand-line embedded string file.

### 8.1 Strategy Decision

- [x] Decide between embedded per-family shader strings and external HLSL assets.
- [ ] If external HLSL is chosen, update package/build scripts to copy or embed shader assets.
- [x] If embedded strings are kept, define source helpers for shader families.
- [x] Document shader cache hash/version implications.

### 8.2 Shader Family Split

- [ ] Split maze/material shader.
- [ ] Split dynamic object shader.
- [ ] Split liquid/effects shader.
- [x] Split overlay shader.
- [x] Split textured overlay shader.
- [x] Split postprocess shader.
- [ ] Split shadow shaders.
- [x] Keep shader entry point names stable.
- [x] Keep input layout creation behavior stable.

### 8.3 Cache and Packaging

- [x] Confirm shader cache file naming remains correct.
- [x] Bump shader cache version only if compiled source or cache assumptions changed.
- [ ] Confirm cache miss compile path works.
- [ ] Confirm cache hit load path works.
- [ ] Confirm release packaging includes any new shader files if applicable.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Shader cold-start path works after clearing relevant cache files.
- [ ] Shader warm-start cached path works.
- [ ] Main menu, playable game, overlays, shadows, and postprocess all render.

Completion notes:

- Agent: Codex
- Date: 2026-06-03
- Strategy chosen: keep embedded HLSL for now. `src/render/renderer_shader_sources.inl` is now a short coordinator that includes the main scene, overlay/textured overlay, and postprocess shader source slices. `CreateShaders` is now an orchestration method; liquid specialization, shader object creation, and input-layout creation live in focused renderer shader helper slices.
- Cache/version changes: no shader cache file naming or package behavior change. The HLSL text is intended to remain unchanged, so no cache version bump is required by this extraction.
- Packaging changes: none; no external shader assets were introduced.

## Phase 9: Convert Stable `.inl` Modules

Goal: replace include-based implementation units with normal C++ translation units once dependencies are explicit.

### 9.1 Low-Risk Candidates

- [x] Convert `src/audio/audio_engine.inl` to `.h/.cpp`.
- [x] Convert `src/maze/maze.inl` fully if not already done.
- [x] Convert `src/audio/game_audio_events.h` event factories to `.cpp`.
- [x] Convert `src/audio/game_audio_system.h` non-template runtime methods to `.cpp`.
- [x] Convert `src/game/player_controller.inl` non-template controller methods to `.cpp`.
- [x] Convert `src/game/player_state.h` method bodies to `.cpp`.
- [x] Convert `src/monster/monster_state.h` method bodies to `.cpp`.
- [x] Convert `src/game/game_session.h` method bodies to `.cpp`.
- [x] Convert low-risk `src/game/game_world.h` method groups to `.cpp`.
- [x] Convert renderer-independent parts of `src/config/settings.inl`.
- [x] Convert stable settings model code from `src/config/config_dialog_model.inl` if dependencies allow.
- [x] Convert global constants from `src/core/constants.inl` to `src/core/constants.h`.
- [x] Convert shared platform include bundle from `src/platform/platform_headers.inl` to `src/platform/platform_headers.h`.
- [x] Convert settings type/binding slices from `.inl` to self-contained headers.
- [x] Convert playable progression type slice from `.inl` to a self-contained header.
- [x] Convert config dialog model type/table slices from `.inl` to guarded headers.
- [x] Convert audio and maze settings adapter slices from `.inl` to inline headers.

### 9.2 Game/App Candidates

- [x] Convert `src/game/game_settings_panel.inl` once UI dependencies are explicit.
- [ ] Convert `src/game/game_shell.inl` once app state dependencies are explicit.
- [x] Split `src/game/game_shell.inl` into focused menu, custom-menu, and input-capture include slices.
- [ ] Convert `src/game/game_app.inl` once platform dependencies are explicit.
- [x] Split `src/game/game_app.inl` into focused window, control-creation, autostart, and loop include slices.
- [ ] Convert `src/screensaver/screensaver_app.inl` only after screensaver smoke coverage is reliable.
- [x] Split `src/screensaver/screensaver_app.inl` into focused mode/window, clone/debug-control, renderer-startup, loop, and selftest include slices.
- [x] Move renderer-independent game window setting application out of `src/game/game_shell.inl`.
- [x] Move loading overlay implementation out of `src/platform/loading_overlay.inl`.
- [x] Move screensaver monitor enumeration out of `src/screensaver/screensaver_app.inl`.
- [x] Move command-line mode parsing out of the old `src/platform/window_proc.inl`.
- [x] Move game/debug command IDs and game/run mode enums out of the old `src/app/app_state.inl`.
- [x] Move config-dialog mode and debug-toolbar helpers out of `src/app/app_runtime.inl` / the old `src/app/app_state.inl`.
- [x] Move game command handling and screensaver quit handling out of the old `src/platform/window_proc.inl`.
- [x] Split target window procedures into explicit game and screensaver include slices.
- [x] Split target-specific `App` host fields and screensaver clone lookup out of `src/app/app_state.inl`.

### 9.3 Renderer Candidates

- [ ] Convert renderer submodules only after their private interfaces are clear.
- [ ] Avoid exposing D3D internals through broad public headers.
- [ ] Keep renderer public API small: initialize, resize, update/render, runtime mode, snapshots/debug hooks.
- [x] Group `renderer.inl` dependency and private implementation includes by ownership.
- [x] Convert `src/render/renderer_dependencies.inl` to `src/render/renderer_dependencies.h`.
- [ ] Remove obsolete `.inl` includes from `renderer.inl` as modules become real translation units.
- [ ] Update `CMakeLists.txt` for each new `.cpp`.

Verification:

- [x] Release build passes.
- [x] `/selftest` passes.
- [ ] Game smoke path passes.
- [ ] Compile errors point to module boundaries rather than include order.
- [ ] `src/main.cpp` remains a thin entry compile shell or becomes a normal entry translation unit.

Completion notes:

- Agent: Codex
- Date: 2026-06-03
- Modules converted: `src/audio/audio_engine.inl` was replaced by `src/audio/audio_engine.h` and `src/audio/audio_engine.cpp`; `src/config/settings.inl` was replaced by `src/config/settings.h` and `src/config/settings.cpp`; `src/config/config_dialog_model.inl` is now a coordinator over dialog constants/types, static field tables, and mode-specific model builders.
- Include-order risks removed: `AudioEngine` no longer depends on the full game `Settings` struct or anonymous-namespace renderer math helpers; renderer code maps settings through `src/audio/audio_engine_settings_adapter.h`. Stable settings metadata and config-dialog model data are isolated from Win32 preview/window-procedure code; config dialog host creation is separated from field parsing, preview, layout, and window procedures.
- Game settings panel slice: `src/game/game_settings_panel.inl` was replaced by `src/game/game_settings_panel.h` and `src/game/game_settings_panel.cpp`. The panel now reports save, key-capture, and close events through `GameSettingsPanelHost`, so the Win32 panel no longer reaches into `gApp` or renderer internals directly. `src/game/game_shell.inl` owns the app-specific callbacks that apply saved settings to input, renderer state, and the game window.
- Settings API slice: `src/config/settings.h` now owns the public settings, image-loading, path, profiling, and runtime-variation declarations; only `src/config/settings.cpp` includes the settings implementation slices. `src/main.cpp`, game sources, the loading overlay, and the game settings panel now include the settings API instead of implementation slices.
- Platform/app slice: `src/platform/loading_overlay.inl` was replaced by `src/platform/loading_overlay.h` and `src/platform/loading_overlay.cpp`; `StartupProgressUpdate`/`StartupProgressSink` moved to `src/debug/startup_progress.h`. `src/game/game_window_settings.cpp` owns fullscreen/window-size application, and `src/screensaver/screensaver_monitor_layout.cpp` owns monitor enumeration.
- App/platform cleanup slice: `RunMode` moved to `src/app/run_mode.h`; `GameState` moved to `src/game/game_state.h`; game command IDs and debug command IDs moved to focused headers. `src/platform/command_line_mode.cpp` now owns screensaver command-line mode parsing and returns debug-start options without mutating debug globals inside the parser. `src/platform/gui_controls.cpp` owns the shared default-GUI-font helper, and `game_app.inl` now uses the existing `MarkerOrEnvEnabled` settings/profile helper instead of a local duplicate.
- Game shell slice: `src/game/game_shell.inl` is now a coordinator over custom menu, main menu, input capture, renderer startup, state transitions, and settings callbacks. Main-menu code is split into layout, presentation, command, and transition slices; custom-game menu code is split into layout, spec, state, adjustment, and command slices; cursor capture and input snapshot collection live in `src/game/game_input_capture.inl`.
- Game app slice: `src/game/game_app.inl` is now a coordinator over window creation, UI setup, autostart, and the message loop. Window placement/class registration, Win32 control creation, autostart profile handling, and the host loop live in focused `src/game/game_app_*.inl` slices.
- App runtime slice: `ConfigDialogMode` moved to `src/config/config_dialog_mode.h`, and debug toolbar title/redraw helpers moved from `src/app/app_state.inl` to `src/debug/debug_slice_controls.inl`.
- Screensaver app slice: `src/screensaver/screensaver_app.inl` is now a coordinator over run-mode setup, primary/clone windows, debug controls, renderer startup, loading warmup, and the playback loop. The selftest entry moved to `src/screensaver/screensaver_self_test.inl`.
- Window procedure slice: `src/platform/window_proc_helpers.inl` now owns only shared window-procedure helpers. `src/game/game_window_proc.inl` and `src/screensaver/screensaver_window_proc.inl` own the target procedures and delegate game resize, paint, cursor, mouse, activation, and `WM_COMMAND` handling to `src/game/game_window_proc_helpers.inl`; debug toolbar creation/commands to `src/debug/debug_slice_controls.inl`; and screensaver clone resize, cursor, mouse, quit-input, activation, and shutdown behavior to `src/screensaver/screensaver_window_proc_helpers.inl` / `src/screensaver/screensaver_quit.inl`. The old `src/platform/window_proc.inl` include was removed after this split.
- App state slice: target-specific fields moved from `src/app/app_state.h` into `src/game/game_app_state_fields.inl` and `src/screensaver/screensaver_app_state_fields.inl`. Screensaver clone lookup moved to `src/screensaver/screensaver_clone_lookup.inl`, leaving app state as shared host fields plus the active app pointer. The old `src/app/app_state.inl` include was replaced by `src/app/app_state.h`.
- Renderer include slice: `src/render/renderer.inl` is now a compact class shell over `src/render/renderer_dependencies.h` and `src/render/renderer_private_modules.inl`. Private renderer includes are grouped into state, gameplay/audio/menu, resources, static scene, and frame/present bundles to make the remaining conversion targets visible.
- Header cleanup slice: settings type/binding declarations, playable progression types, config dialog model types/tables, audio settings adapters, maze settings adapters, and renderer dependency includes were promoted from `.inl` slices to guarded headers where they are declaration or inline-adapter code.
- Renderer texture slice: `src/render/renderer_textures.inl` is now the material-atlas creation coordinator. Loose-page texture loading, runtime/custom-menu texture upload, and high-res PBR texture loading moved to focused renderer texture slices.
- Renderer mesh-loading slice: `src/render/renderer_mesh_loading.inl` is now a coordinator over asset path/cache helpers, monster mesh loading, static prop mesh loading, and prop-library loading slices.
- Renderer placement slice: static prop placement, liquid/decal placement, water-surface placement, and blood-damage placement are now short coordinators over focused helper slices. This leaves the remaining large renderer placement work concentrated in the genuinely dense generation bodies instead of broad mixed-responsibility files.
- Config dialog slice: config field persistence, preview orbit input, parsing helpers, settings reconstruction, preview window procedure, scroll-pane window procedure, main window creation, and main message routing now live in focused slices under the config dialog coordinator.
- Player camera/navigation slice: camera attention and path scoring are now coordinators over room/branch attention, corridor scanning, prop/visibility attention, exit attention, base path scoring, corridor continuation, threat-risk scoring, and threat-escape helpers.
- Include coordinator cleanup slice: settings file I/O, runtime texture upload, device resources, water-like damage placement, maze surface generation, main-menu scene updates, monster/static-prop mesh loading, static scene finalization, renderer startup, renderer menu scene helpers, furniture/fixture placement, dynamic effect geometry, gameplay audio events, and player session reset/restart were split into focused include slices. `config_dialog_settings_reconstruction.inl` now only owns `SettingsFromConfigControls`; duplicate preview-orbit and control-parsing definitions were removed from that file.
- Post-checkpoint cleanup slice: texture-cache hashing/path/load/save, lamp spark scare events, renderer menu lifecycle, maze placement helpers, player blood-pressure updates, and blood damage overlay placement were split into focused include slices.
- Risky-file split slice: `player_autopilot_path_follower.inl` is now a coordinator over state timers, sight freeze, scan pauses, path acquisition, free-run targeting, path advancement, look composition, turn/pitch, and movement/camera slices. The current monster presentation files now use the monster name: dynamic body rendering moved from generic monster geometry naming to `renderer_omukade_geometry*.inl`, with `AppendOmukadeGeometry` as the append entry. Omukade body-chain sampling, surface attachment, constraints, limb reach targets, and limb geometry were split from the largest Omukade body files.
- Menu/settings constant split slice: `renderer_custom_menu_texture.inl` now coordinates GDI setup, page routing, environment detail, scare detail, root page, and GPU upload slices. `player_main_menu_scene_update.inl` now coordinates start transition, idle camera, custom-view camera, and menu scene effects. `renderer_present_constants_lighting.inl` now coordinates fixture-shadow selection, lighting/post constants, and shadow/maze/texture/transition constants. `settings_load.inl` now coordinates section-specific INI readers.
- Shader chunk slice: the embedded main-scene HLSL remains embedded but is split into small raw-string chunks under `renderer_main_shader_chunk_*.inl`, avoiding MSVC raw-string and string-literal size limits while preserving stable shader entry points.
- Final include-slice push: config dialog message routing/settings reconstruction, default INI sections, flashlight pose, renderer startup/update, dynamic particles, mesh path/cache parsing, water pool helpers, game window procedure helpers, debug cameras, wet audio, furniture scatter, scare-event families, monster movement/targeting/update, playable progression, GPU profiling, and texture-cache hashing were split into focused coordinators where boundaries were clean. Two dense render builders, maze mesh construction and water damage placement, were left as safe body/whole-function includes after finer line-range cuts proved too fragile.
- Continuation include-slice push: exit-doorway geometry, present visibility helpers, Omukade head presentation, liquid canvas helpers, air-particle runtime, water wall spills, dynamic primitives, HUD notification drawing, debug-slice runtime, custom-game menu layout, floor-footprint placement, high-res PBR texture helpers, runtime-variation helpers, and profiling/logging helpers were split by function or explicit branch boundaries. The remaining largest files are now concentrated in dense single-function render builders and texture/material generators.
- Aggressive continuation slice: map overlay rendering, blood-damage batch placement, static prop OBJ loading, native Omukade mask loading, texture atlas/PBR helper setup, procedural material generation, autopilot path/attention composition, config model building, dynamic menu geometry, maze surface helpers, water tile cards, liquid wall helpers/canvas, custom-menu GDI setup, prop debug inspection, shadow pass staging, horror constants, monster update coordination, high-res/loose-page texture loading, lifecycle/runtime textures, blood dread reactions, Omukade visibility/body/surface rendering, path topology/threat helpers, audio occlusion/lamp runtime, and autopilot path choice were split into focused include slices. After this pass, only `renderer_maze_mesh_body.inl` and `renderer_water_damage_placement.inl` remain above 120 lines; most remaining large files are now narrow helpers around 100-118 lines.
- Final aggressive split slice: the two formerly largest render builders, water-damage placement and maze mesh construction, were split into helper includes. Follow-up batches split screensaver renderer startup, debug toolbar controls, settings asset loading, present constants, player session transitions, HUD notification texture upload, liquid fit helpers, exit-door props, game host controls, scare particle updates, effect debug globals, custom-game commands, monster focus/preview camera, chase panic, present visibility maze helpers, config dialog layout/host, static index chunking, prop mesh libraries, wall vents, Omukade mask loading, main-menu custom camera view, wet-floor audio, collision movement fallback, monster lamp damage, lifecycle frame profiling, loose-paper props, lamp placement, monster hearing, Omukade limb reach, exit/fog constants, air-particle billboards, metal cabinets, menu plaques, and queued game-audio helpers. Ignoring embedded shader literal chunks, the largest remaining `.inl` files are now about 80 lines and are mostly focused single-purpose helpers.
- Module conversion / final trimming slice: `src/debug/effect_debug.inl` was replaced by `src/debug/effect_debug.h` and `src/debug/effect_debug.cpp`, moving debug globals and prop/effect labels out of `main.cpp` and into a normal translation unit. Additional cleanup split water-wall pool merging, water/leak cards, exit-portal wall runs, menu label atlas drawing, game/screen saver message loops, game control creation, present shadow/maze constants, shader specialization, Omukade body/contact helpers, game renderer startup, input capture, autopilot path/movement, HUD drawing, manual-controller bridge, runtime variation, room survey setup, and instanced static prop submission. Ignoring embedded shader literal chunks, the largest remaining `.inl` files are now about 74 lines.
- Settings module / tail cleanup slice: the catch-all `src/config/settings.cpp` was replaced by focused settings translation units for assets, defaults, file I/O, input/runtime updates, paths, profiles, and runtime variation. Follow-up include cleanup split config parsing, game settings transitions, instanced static registration, renderer pipeline state creation, Omukade mask loading, liquid canvas marking, PBR material loading, water/material helpers, small furniture props, game-audio event dispatch, GPU profile resolution, lamp placement, menu commands, player exit attention, air-particle respawn, static prop append helpers, and debug-slice settings application into smaller named slices.
- Final module split slice: `src/game/game_world.cpp` is now a thin anchor over focused translation units for session snapshots, maze/save persistence, player state, player navigation, monster state, playable progression, collectible pages, audio events, save points, lifecycle transitions, and collectible generation. `src/platform/loading_overlay.cpp` was split behind a private loading-overlay internal header into state, assets, drawing, window, thread, and public API files. The previous catch-all `playable_progression_types.h` is now a small aggregator over custom-game spec, level spec, progression result/saved-run helpers, and run-state headers. `src/game/game_settings_panel.cpp` was split behind a private panel internal header into persistence, drawing, interaction, window, and API files.
- Final header/module closure slice: `PlayableRunState`, `PlayerController`, `GameWorld`, `AudioEngine`, render shared types, settings fields, and config-dialog field tables are now compact coordinators over focused type/API/data slices. `src/audio/audio_engine.cpp`, `src/maze/maze.cpp`, and `src/game/player_controller.cpp` are thin anchors over focused translation units.
- Final implementation closure slice: the remaining largest dense bodies were split where boundaries were still mechanical and safe. Branded loading-overlay drawing now delegates mark, title, progress, detail, and spinner work to focused translation units; game settings panel painting is separate from primitive control drawing; collectible page aiming is separate from collectible generation; and audio voice selection/startup/3D/spatialization are separate translation units.
- Verification note: Release build passed after the settings API and game settings panel conversions. Later Release builds also passed after the loading-overlay, monitor-layout, command-line, game-shell include-slice, game-app include-slice, app-runtime debug/config cleanup, screensaver include-slice, window-proc helper extraction, header cleanup, shader split, game-shell/menu split, renderer texture split, renderer mesh-loading split, aggressive include-split cleanup, post-checkpoint cleanup, Omukade/autopilot risky-file split, custom-menu/present-constant split, settings-load split, final include-slice push, continuation include-slice push, aggressive continuation batches, water/maze builder split, final 80-line cleanup batches, `effect_debug.cpp` conversion, final 74-line cleanup batches, the settings module / tail cleanup slice, the final module split slice, the final header/module closure slice, and the final implementation closure slice. `BackroomsMazeTest.exe /selftest` returned exit code 0 after both the aggressive include-split cleanup and post-checkpoint cleanup batches, again after the settings module / tail cleanup slice, again after the final module split slice, again after the final header/module closure slice, and again after the final implementation closure slice.
- Remaining `.inl` modules: `src/config/config_dialog.inl` remains include-based but is now a thin coordinator over controls, preview, layout, window procedures, and host creation. `src/game/game_shell.inl`, `src/game/game_app.inl`, and `src/screensaver/screensaver_app.inl` are still include-based because they depend directly on `gApp`, Win32 message ordering, and shell helpers, but their large runtime groups have been split into focused slices. Renderer internals still need deeper dependency cleanup before full `.cpp` conversion. The remaining larger normal files are focused modules rather than broad mixed-responsibility files; further work should target explicit gameplay/render ownership boundaries and manual smoke coverage, not more line-count-driven splitting.

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
