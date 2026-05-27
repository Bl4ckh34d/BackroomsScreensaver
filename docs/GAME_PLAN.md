# Backrooms Maze Game Plan

This file is the source of truth for turning the screensaver into a playable C++ game while keeping the screensaver stable.

## Current Status

- [x] Existing screensaver remains the baseline visual/atmosphere implementation.
- [x] Add a separate `BackroomsMazeGame.exe` target.
- [x] Add first game shell states: Main Menu, Single Player, Settings, Debug, Exit.
- [x] Add first-pass manual player controls and HUD.
- [x] Split game settings into System, Graphics, Game, Controls, and Audio views.
- [x] Add Debug-only settings view for overlays, effect tuning, autopilot tuning, and monster calibration.
- [ ] Expand Debug into a full in-game test lab.
- [ ] Add stealth/noise gameplay.
- [ ] Replace placeholder monster with a procedural many-limbed creature.
- [ ] Add full sound design.

## MVP Checklist

- [x] Main Menu with Single Player, Settings, Debug, and Exit.
- [x] Keep Settings wired to the existing configuration UI.
- [x] Filter game Settings to game-relevant categories instead of showing the full screensaver configuration.
- [x] Add Debug Settings inside Debug for debug/test-only controls.
- [x] Keep Debug wired to the existing effect/prop debug slice viewer.
- [x] Single Player starts a playable maze run.
- [x] Esc returns from Play or Debug to Main Menu.
- [x] WASD movement.
- [x] Mouse look.
- [x] Space jump with simple gravity.
- [x] Shift sprint with stamina drain and regeneration.
- [x] Ctrl/C crouch placeholder with lower eye height and gentler bob.
- [x] E interaction with the exit door when close and facing it.
- [x] Health and stamina bars.
- [ ] Add in-game text labels/tooltips once a font/text renderer exists.

## Player Roadmap

- Current controls settings include mouse sensitivity and invert Y.
- Walking should create normal footstep noise.
- Running should drain stamina and greatly increase monster hearing distance.
- Crouching is a light stance change, not a full stealth simulation at first: lower camera height, slower speed, gentler vertical bob, almost no side bob.
- Crouching should reduce footstep hearing distance enough that a monster in the next hallway will not immediately update its target.
- Health should support monster damage/death first, then later environmental damage or sanity/dread effects if useful.
- E interaction should expand from exit door only to props, debug triggers, doors, switches, and scripted scares.

## Monster Roadmap

- Reuse the existing autopilot/pathfinding ideas for monster movement where possible.
- Monster should be attracted to player-created noise: footsteps, running, thrown/interactive objects, sparks, vents, and other trigger events.
- Current monster remains a placeholder until the playable shell is stable.
- Target creature: scary face filling most of the hallway, with a centipede/snake body moving through the maze.
- Procedural animation matters more than mesh complexity.
- Body should follow labyrinth turns with segmented motion.
- Many hands/limbs should search for nearby wall/floor/ceiling contact points.
- Each limb needs an outer range where it starts preparing/repositioning and a closer range where it reaches, attaches, grabs, or pushes.
- Eyes and face should track the player when spotted.
- When the player is not spotted, eyes/face should scan around.
- Behavior should include pauses to look around, sudden fast movement through local areas, searching last-known positions, and roaming.
- Materials can be simple at first: matte bone color plus glossy blood/wetness.

## Audio Roadmap

- Current Audio settings are persisted in the INI but not consumed yet because the audio engine is not implemented.
- Player footsteps need dry carpet and moist carpet variants.
- Monster many-feet movement should be audible beyond viewing distance.
- Lamps need buzzing and unstable flicker sounds.
- Broken lamps need spark crackle.
- Air vents need pressure puffs/booms.
- Vents can carry distant eerie screeches when the player passes nearby.
- Water damage and blood should have localized dripping/wet sounds.
- Audio should feed gameplay: loud events can update monster targets.

## Debug Scene Roadmap

- Existing debug slice viewer is the first Debug state.
- Debug Settings currently owns map/dread overlays, fixed study views, effect loop tuning, autopilot camera tuning, and monster mesh/eye calibration.
- Add texture/material test wall.
- Add model browser for runtime props and monster prototypes.
- Add labyrinth generation test controls.
- Add gameplay trigger test area for sparks, lamp flicker, vents, blood, and wall bleeding.
- Add monster AI test controls for sight, sound target, chase, search, roam, and kill distance.
- Add shader/effect stress tests and performance counters.
- Keep Debug back-and-forward navigable from the game menu.

## Agent Handoff Rules

- Keep `BackroomsMaze.scr` behavior stable when changing game code.
- Prefer adding explicit runtime modes over mixing screensaver and game behavior.
- Keep game states simple and easy to expand.
- Reuse existing renderer, maze, effect, prop, and monster systems before adding new dependencies.
- Do not add PhysX or heavy physics until simple collision/jump gameplay proves insufficient.
- Do not stage `BackroomsMaze_backup.ini` unless it has a real content diff.
- Before committing: build Release, run `/selftest`, and smoke-test `BackroomsMazeGame.exe`.
