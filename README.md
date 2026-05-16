# Backrooms Maze Screensaver

Native Win32 screensaver built with Direct3D 11, using feature level 10.0+ shaders.

## Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```

Output:

```text
build\Release\BackroomsMaze.scr
```

## Test

Hidden Direct3D smoke test:

```powershell
Copy-Item .\build\Release\BackroomsMaze.scr .\build\Release\BackroomsMazeTest.exe -Force
Start-Process .\build\Release\BackroomsMazeTest.exe -ArgumentList "/selftest" -Wait -PassThru
```

Windows treats `.scr` files specially, so use the `.exe` copy for automated command-line tests.

Fullscreen screensaver preview:

```powershell
.\build\Release\BackroomsMaze.scr /s
```

Move the mouse or press a key to exit.

Effect slice debug viewer:

```powershell
.\build\Release\BackroomsMazeBloodDebug.exe /effectdebug
```

Use `< Effect`, `Effect >`, and `Grid: NxN` to inspect blood, water, ceiling lamps, broken lamps, and air vents in a well-lit 1x1 through 5x5 map slice. `/blooddebug` opens the same viewer starting on blood.

## Install

Right-click `build\Release\BackroomsMaze.scr` and choose **Install**.

## Configure

Right-click `build\Release\BackroomsMaze.scr` and choose **Configure**.

This opens a grouped settings window with tabs for renderer, maze, textures, lighting, and camera AI. A live Direct3D preview runs on the right and refreshes shortly after field changes. Use **Save** to write the settings to:

- `build\Release\BackroomsMaze.ini`

The settings expose maze size/room generation, PBR texture folder/stems, texture scale, flashlight brightness/shadow controls, lamp spacing/on/off/flicker/broken-zone ratios, fog, ambient occlusion, exposure/gamma, and camera AI timing/speeds/scan behavior. Leave `FloorStem` or `CeilingStem` empty to use the built-in procedural floor/ceiling textures. Set `CeilingScaleMeters=0` to keep the ceiling plate texture aligned as a 2x2 panel grid per maze tile.

`[Renderer] AllowWarpFallback=0` keeps rendering on the hardware GPU. Set it to `1` only if you want Direct3D WARP software fallback when no hardware D3D device is available.

## Current Features

- Direct3D 11 renderer targeting feature level 10.0 or newer.
- Win32 screensaver modes: `/s`, `/c`, `/p <HWND>`.
- Loads PBR maps from `assets\PBRs` when present, with procedural fallback textures.
- Uses Backrooms-tuned wall, stained carpet, and office ceiling PBRs by default.
- Uses color, DirectX normal, and height maps for parallax-style material sampling.
- Adds procedural exit-door geometry with an illuminated emergency exit sign.
- Scatters low-poly office chairs, waiting-room chairs, loose paper piles, water stains, ceiling damage, and dripping leaks through the maze.
- Uses 3D ceiling light panels with frames and fluorescent bulb strips.
- Dark fog and a steady forward flashlight.
- Shadow-mapped flashlight pass for walls and mesh props such as chairs, door frames, and lamp housings.
- Buzzing office ceiling light panels, randomized fixture flicker, and large broken-light zones.
- Sparse localized ceiling light emission rather than a global glow.
- Dimmer steady flashlight with no pulse.
- Camera head bob and side sway for a walking feel.
- Larger procedural maze generation with bigger room cuts.
- Autopilot pathing with exit visibility checks, turn look-ahead, room/junction pauses, look-around sweeps, occasional look-behind stops, and flee behavior.
- One chasing alpha-billboard creature sprite.

## Assets

The renderer looks for PBR files in:

- `assets\PBRs` beside the project
- `assets\PBRs` beside the `.scr`
- `assets\PBRs` one or two levels above the `.scr`

That lets `build\Release\BackroomsMaze.scr` use the project-root `assets\PBRs` folder directly.

The bundled Backrooms materials are normalized into the `backrooms_*` stems from ambientCG source assets: `Wallpaper002C`, `Paint006`, `Fabric028`, and `OfficeCeiling001`.
