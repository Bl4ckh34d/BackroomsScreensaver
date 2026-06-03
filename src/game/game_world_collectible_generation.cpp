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

void GameWorld::GenerateCollectiblePagesForCurrentLevel(bool enabled, float wallHeightMeters, std::mt19937& rng) {
        ResetCollectiblePagesForGeneration();
        if (!enabled || maze.open.empty()) return;

        int firstPageIndex = 0;
        int pageCount = kCollectiblePageMaterialCount;
        if (playableRun.active) {
            playableRun.EnsureLayerPageDistribution();
            firstPageIndex = playableRun.LayerPageStartForLevel(playableRun.levelInLayer);
            pageCount = playableRun.LayerPageCountForLevel(playableRun.levelInLayer);
        }
        if (pageCount <= 0) return;

        auto randRange = [&rng](float a, float b) {
            std::uniform_real_distribution<float> dist(a, b);
            return dist(rng);
        };
        auto scale = [](XMFLOAT3 a, float s) {
            return XMFLOAT3{a.x * s, a.y * s, a.z * s};
        };
        auto add = [](XMFLOAT3 a, XMFLOAT3 b) {
            return XMFLOAT3{a.x + b.x, a.y + b.y, a.z + b.z};
        };
        auto cross = [](XMFLOAT3 a, XMFLOAT3 b) {
            return XMFLOAT3{
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            };
        };
        auto dot = [](XMFLOAT3 a, XMFLOAT3 b) {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        };
        auto normalize = [&scale, &dot](XMFLOAT3 a, XMFLOAT3 fallback) {
            float len = std::sqrt(std::max(0.0f, dot(a, a)));
            if (len <= 0.00001f) return fallback;
            return scale(a, 1.0f / len);
        };

        std::vector<Tile> openTiles;
        openTiles.reserve(static_cast<size_t>(maze.w * maze.h));
        auto collectibleFloorTile = [this](Tile t) {
            return maze.IsOpen(t.x, t.y) && maze.WallFeature(t.x, t.y) == MazeWallFeature::None;
        };
        auto collectibleWallNeighbor = [this](Tile t) {
            return maze.InBounds(t.x, t.y) &&
                !maze.IsOpen(t.x, t.y) &&
                maze.WallFeature(t.x, t.y) == MazeWallFeature::None;
        };
        for (int y = 1; y < maze.h - 1; ++y) {
            for (int x = 1; x < maze.w - 1; ++x) {
                Tile t{x, y};
                if (!collectibleFloorTile(t) || t == maze.start || t == maze.exit) continue;
                if (std::abs(x - maze.start.x) + std::abs(y - maze.start.y) < 4) continue;
                openTiles.push_back(t);
            }
        }
        if (openTiles.empty()) return;

        std::array<Tile, kCollectiblePageMaterialCount> usedTiles{};
        int usedCount = 0;
        auto tileUsed = [&](Tile t) {
            for (int i = 0; i < usedCount; ++i) {
                if (usedTiles[static_cast<size_t>(i)] == t) return true;
            }
            return false;
        };
        auto rememberTile = [&](Tile t) {
            if (usedCount < static_cast<int>(usedTiles.size())) {
                usedTiles[static_cast<size_t>(usedCount++)] = t;
            }
        };

        constexpr float pageH = 0.297f;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int pageOffset = 0; pageOffset < pageCount; ++pageOffset) {
            int pageIndex = firstPageIndex + pageOffset;
            if (pageIndex < 0 || pageIndex >= kCollectiblePageMaterialCount) continue;
            if (playableRun.active && IsLayerPageCollected(pageIndex)) continue;

            bool placed = false;
            for (int attempt = 0; attempt < 220 && !placed; ++attempt) {
                Tile t = openTiles[static_cast<size_t>(rng() % openTiles.size())];
                if (tileUsed(t) && attempt < 96) continue;
                XMFLOAT3 tileCenter = maze.WorldCenter(t, 0.0f);
                bool preferWall = randRange(0.0f, 1.0f) < 0.68f;
                if (preferWall) {
                    std::array<int, 4> order{{0, 1, 2, 3}};
                    std::shuffle(order.begin(), order.end(), rng);
                    for (int ord : order) {
                        int dx = dirs[ord][0];
                        int dy = dirs[ord][1];
                        if (!collectibleWallNeighbor({t.x + dx, t.y + dy})) continue;
                        XMFLOAT3 normal{static_cast<float>(-dx), 0.0f, static_cast<float>(-dy)};
                        normal = normalize(normal, {0.0f, 0.0f, 1.0f});
                        XMFLOAT3 right = normalize(cross({0.0f, 1.0f, 0.0f}, normal), {1.0f, 0.0f, 0.0f});
                        float lateral = randRange(-0.36f, 0.36f) * ((dx != 0) ? maze.tileD : maze.tileW);
                        XMFLOAT3 wallCenter = tileCenter;
                        wallCenter.x += static_cast<float>(dx) * maze.tileW * 0.5f;
                        wallCenter.z += static_cast<float>(dy) * maze.tileD * 0.5f;
                        wallCenter = add(wallCenter, scale(normal, 0.010f));
                        wallCenter = add(wallCenter, scale(right, lateral));
                        wallCenter.y = std::clamp(1.38f + randRange(-0.22f, 0.20f), pageH * 0.5f + 0.08f, wallHeightMeters - pageH * 0.5f - 0.05f);
                        XMFLOAT3 baseRight = right;
                        XMFLOAT3 baseUp{0.0f, 1.0f, 0.0f};
                        float roll = randRange(-0.115f, 0.115f);
                        float cr = std::cos(roll);
                        float sr = std::sin(roll);
                        right = normalize(add(scale(baseRight, cr), scale(baseUp, sr)), baseRight);
                        XMFLOAT3 pageUp = normalize(add(scale(baseUp, cr), scale(baseRight, -sr)), baseUp);
                        PlaceCollectiblePage(pageIndex, {wallCenter, right, pageUp, normal, pageIndex, false});
                        rememberTile(t);
                        placed = true;
                        break;
                    }
                }
                if (!placed) {
                    float px = tileCenter.x + randRange(-0.32f, 0.32f) * std::max(0.1f, maze.tileW - pageH);
                    float pz = tileCenter.z + randRange(-0.32f, 0.32f) * std::max(0.1f, maze.tileD - pageH);
                    float yaw = randRange(0.0f, kPi * 2.0f);
                    XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                    XMFLOAT3 up{std::sin(yaw), 0.0f, std::cos(yaw)};
                    PlaceCollectiblePage(pageIndex, {{px, 0.006f + pageIndex * 0.00015f, pz}, right, up, {0.0f, 1.0f, 0.0f}, pageIndex, false});
                    rememberTile(t);
                    placed = true;
                }
            }
        }
    }
