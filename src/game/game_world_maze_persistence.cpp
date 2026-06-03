#include "../platform/platform_headers.h"
#include "../core/maze_types.h"
#include "../core/math_utils.h"
#include "../core/constants.h"
#include "../debug/effect_debug_constants.h"
#include "../config/settings.h"
#include "../audio/audio_engine.h"
#include "../audio/game_audio_events.h"
#include "../maze/maze.h"
#include "../gameplay/playable_progression_types.h"
#include "../monster/monster_state.h"

#include "player_controller.h"
#include "player_state.h"
#include "game_world.h"

void GameWorld::ApplyMazeLayout(const MazeLayoutSpec& spec, bool updateExit) {
    maze.w = spec.width;
    maze.h = spec.height;
    maze.tileW = spec.tileW;
    maze.tileD = spec.tileD;
    if (updateExit) maze.exit = {maze.w - 2, maze.h - 2};
}

void GameWorld::GenerateMaze(const GameWorldMazeGenerationRequest& request) {
    maze.rng.seed(request.runtimeSeed);
    if (request.applyLayout) {
        ApplyMazeLayout(request.layout, request.updateExit);
    }

    switch (request.kind) {
    case GameWorldMazeGenerationKind::MainMenu:
        maze.GenerateMenuRoom();
        break;
    case GameWorldMazeGenerationKind::DebugSlice:
        maze.GenerateDebugSlice(request.debugSliceTiles);
        break;
    case GameWorldMazeGenerationKind::BloodDebugCorridor:
        maze.GenerateBloodDebugCorridor();
        break;
    case GameWorldMazeGenerationKind::BenchmarkDemo:
        maze.GenerateBenchmarkDemo();
        break;
    case GameWorldMazeGenerationKind::Standard:
    default:
        maze.Generate(request.generation);
        break;
    }
}

const Maze& GameWorld::MazeView() const {
    return maze;
}

std::wstring GameWorld::EncodeSavedMazeBytes(const std::vector<uint8_t>& bytes) {
    std::wstring out;
    out.reserve(bytes.size());
    for (uint8_t v : bytes) out.push_back(static_cast<wchar_t>(L'0' + std::clamp<int>(v, 0, 9)));
    return out;
}

void GameWorld::DecodeSavedMazeBytes(const std::wstring& text, std::vector<uint8_t>& out, size_t expectedSize) {
    out.assign(expectedSize, 0);
    size_t count = std::min(expectedSize, text.size());
    for (size_t i = 0; i < count; ++i) {
        wchar_t ch = text[i];
        out[i] = (ch >= L'0' && ch <= L'9') ? static_cast<uint8_t>(ch - L'0') : 0;
    }
}

void GameWorld::WriteSavedRunFields(std::wostream& out) const {
    out << L"MazeW=" << maze.w << L"\n";
    out << L"MazeH=" << maze.h << L"\n";
    out << L"MazeTileW=" << maze.tileW << L"\n";
    out << L"MazeTileD=" << maze.tileD << L"\n";
    out << L"StartX=" << maze.start.x << L"\n";
    out << L"StartY=" << maze.start.y << L"\n";
    out << L"ExitX=" << maze.exit.x << L"\n";
    out << L"ExitY=" << maze.exit.y << L"\n";
    out << L"Open=" << EncodeSavedMazeBytes(maze.open) << L"\n";
    out << L"WallFeatures=" << EncodeSavedMazeBytes(maze.wallFeatures) << L"\n";
    out << L"CameraX=" << player.position.x << L"\n";
    out << L"CameraY=" << player.position.y << L"\n";
    out << L"CameraZ=" << player.position.z << L"\n";
    out << L"Yaw=" << player.yaw << L"\n";
    out << L"BodyYaw=" << player.bodyYaw << L"\n";
    out << L"LookPitch=" << player.pitch << L"\n";
    out << L"PlayerHealth=" << player.health << L"\n";
    out << L"PlayerStamina=" << player.stamina << L"\n";
    out << L"CollectedPages=" << collectiblePagesCollected << L"\n";
    out << L"SavePointActive=" << (savePoint.active ? 1 : 0) << L"\n";
    out << L"SavePointX=" << savePoint.pos.x << L"\n";
    out << L"SavePointY=" << savePoint.pos.y << L"\n";
    out << L"SavePointZ=" << savePoint.pos.z << L"\n";
    out << L"SavePointYaw=" << savePoint.yaw << L"\n";
    for (size_t i = 0; i < collectiblePages.size(); ++i) {
        out << L"Page" << i << L"Collected=" << (collectiblePages[i].collected ? 1 : 0) << L"\n";
    }
}

GameWorldSavedMazeRestoreState GameWorld::ReadSavedMazeRestoreState(
    const SavedRunKeyValues& values,
    const PlayableLevelSpec& spec,
    const MazeLayoutSpec& fallbackLayout) const {
    GameWorldSavedMazeRestoreState saved{};
    saved.w = std::clamp(SavedRunInt(values, L"MazeW", spec.mazeWidth), 3, 151);
    saved.h = std::clamp(SavedRunInt(values, L"MazeH", spec.mazeHeight), 3, 151);
    saved.tileW = std::clamp(SavedRunFloat(values, L"MazeTileW", fallbackLayout.tileW), 1.2f, 8.0f);
    saved.tileD = std::clamp(SavedRunFloat(values, L"MazeTileD", fallbackLayout.tileD), 1.2f, 8.0f);
    saved.start = {
        std::clamp(SavedRunInt(values, L"StartX", 1), 0, saved.w - 1),
        std::clamp(SavedRunInt(values, L"StartY", 1), 0, saved.h - 1)
    };
    saved.exit = {
        std::clamp(SavedRunInt(values, L"ExitX", saved.w - 2), 0, saved.w - 1),
        std::clamp(SavedRunInt(values, L"ExitY", saved.h - 2), 0, saved.h - 1)
    };
    saved.open = SavedRunString(values, L"Open");
    saved.wallFeatures = SavedRunString(values, L"WallFeatures");
    return saved;
}

void GameWorld::RestoreSavedMazeGeometry(const GameWorldSavedMazeRestoreState& saved) {
    maze.w = saved.w;
    maze.h = saved.h;
    maze.tileW = saved.tileW;
    maze.tileD = saved.tileD;
    maze.start = saved.start;
    maze.exit = saved.exit;
    const size_t cellCount = static_cast<size_t>(maze.w * maze.h);
    DecodeSavedMazeBytes(saved.open, maze.open, cellCount);
    DecodeSavedMazeBytes(saved.wallFeatures, maze.wallFeatures, cellCount);
    if (!maze.IsOpen(maze.start.x, maze.start.y)) maze.SetOpen(maze.start.x, maze.start.y);
    if (!maze.IsOpen(maze.exit.x, maze.exit.y)) maze.SetOpen(maze.exit.x, maze.exit.y);
}

GameWorldSavedRuntimeRestoreState GameWorld::ReadSavedRuntimeRestoreState(const SavedRunKeyValues& values) const {
    GameWorldSavedRuntimeRestoreState saved{};
    XMFLOAT3 defaultPlayer = maze.WorldCenter(maze.start, 1.45f);
    saved.playerPosition = {
        SavedRunFloat(values, L"CameraX", defaultPlayer.x),
        SavedRunFloat(values, L"CameraY", 1.45f),
        SavedRunFloat(values, L"CameraZ", defaultPlayer.z)
    };
    saved.playerYaw = SavedRunFloat(values, L"Yaw", 0.0f);
    saved.playerBodyYaw = SavedRunFloat(values, L"BodyYaw", saved.playerYaw);
    saved.playerPitch = SavedRunFloat(values, L"LookPitch", -0.055f);
    saved.playerHealth = SavedRunFloat(values, L"PlayerHealth", 100.0f);
    saved.playerStamina = SavedRunFloat(values, L"PlayerStamina", 100.0f);
    saved.collectedPages = SavedRunInt(values, L"CollectedPages", 0);
    saved.savePoint.active = SavedRunInt(values, L"SavePointActive", savePoint.active ? 1 : 0) != 0;
    saved.savePoint.pos = {
        SavedRunFloat(values, L"SavePointX", savePoint.pos.x),
        SavedRunFloat(values, L"SavePointY", savePoint.pos.y),
        SavedRunFloat(values, L"SavePointZ", savePoint.pos.z)
    };
    saved.savePoint.yaw = SavedRunFloat(values, L"SavePointYaw", savePoint.yaw);
    for (size_t i = 0; i < saved.pageCollected.size(); ++i) {
        std::wstring key = L"Page" + std::to_wstring(i) + L"Collected";
        bool fallback = i < collectiblePages.size() && collectiblePages[i].collected;
        saved.pageCollected[i] = SavedRunInt(values, key.c_str(), fallback ? 1 : 0) != 0 ? 1 : 0;
    }
    return saved;
}

void GameWorld::RestoreSavedRuntimeState(const GameWorldSavedRuntimeRestoreState& saved) {
    player.RestoreSavedRunState(
        saved.playerPosition,
        saved.playerYaw,
        saved.playerBodyYaw,
        saved.playerPitch,
        saved.playerHealth,
        saved.playerStamina);
    RestoreSavePoint(saved.savePoint.active, saved.savePoint.pos, saved.savePoint.yaw);
    for (size_t i = 0; i < saved.pageCollected.size(); ++i) {
        RestoreCollectiblePageCollected(i, saved.pageCollected[i] != 0, true);
    }
    ReconcileCollectiblePageRestore(saved.collectedPages);
}
