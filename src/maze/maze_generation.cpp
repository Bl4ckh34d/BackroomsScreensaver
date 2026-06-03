// Maze layout generation and navigation queries.

#include "../platform/platform_headers.h"
#include "maze.h"

void Maze::GenerateWallFeatures(const MazeGenerationSpec& spec) {
        wallFeatures.assign(static_cast<size_t>(w * h), 0);

        std::vector<Tile> candidates;
        candidates.reserve(static_cast<size_t>(w * h / 12));
        for (int y = 1; y < h - 1; ++y) {
            for (int x = 1; x < w - 1; ++x) {
                if (IsOpen(x, y)) continue;
                if (std::abs(x - start.x) + std::abs(y - start.y) <= 3 ||
                    std::abs(x - exit.x) + std::abs(y - exit.y) <= 3) {
                    continue;
                }

                const bool east = IsOpen(x + 1, y);
                const bool west = IsOpen(x - 1, y);
                const bool south = IsOpen(x, y + 1);
                const bool north = IsOpen(x, y - 1);
                const bool eastWestPassage = east && west && !south && !north;
                const bool northSouthPassage = south && north && !east && !west;
                if (eastWestPassage || northSouthPassage) candidates.push_back({x, y});
            }
        }
        if (candidates.empty()) return;

        const float baseFrequency = std::clamp(static_cast<float>(spec.wallFeatureFrequency), 1.0f, 200.0f);
        const float spread = std::clamp(spec.wallFeatureFrequencySpread, 0.0f, 3.0f);
        std::uniform_real_distribution<float> spreadDist(-spread, spread);
        const float frequency = std::clamp(static_cast<float>(baseFrequency * std::pow(2.0f, spreadDist(rng))), 1.0f, 200.0f);
        int target = std::clamp(static_cast<int>(std::round(static_cast<float>(candidates.size()) / frequency)),
            1, static_cast<int>(candidates.size()));

        std::shuffle(candidates.begin(), candidates.end(), rng);
        for (int i = 0; i < target; ++i) {
            const Tile t = candidates[static_cast<size_t>(i)];
            wallFeatures[static_cast<size_t>(t.y * w + t.x)] =
                (rng() & 1u) ? static_cast<uint8_t>(MazeWallFeature::Window) : static_cast<uint8_t>(MazeWallFeature::Tunnel);
        }
    }

void Maze::Generate(const MazeGenerationSpec& spec) {
        open.assign(static_cast<size_t>(w * h), 0);
        wallFeatures.assign(static_cast<size_t>(w * h), 0);
        start = {1, 1};
        std::vector<Tile> stack;
        stack.push_back(start);
        SetOpen(start.x, start.y);
        const std::array<Tile, 4> dirs = {{{2, 0}, {-2, 0}, {0, 2}, {0, -2}}};

        while (!stack.empty()) {
            Tile cur = stack.back();
            std::array<Tile, 4> shuffled = dirs;
            std::shuffle(shuffled.begin(), shuffled.end(), rng);
            bool carved = false;
            for (Tile d : shuffled) {
                Tile nxt{cur.x + d.x, cur.y + d.y};
                if (nxt.x <= 0 || nxt.y <= 0 || nxt.x >= w - 1 || nxt.y >= h - 1 || IsOpen(nxt.x, nxt.y)) {
                    continue;
                }
                SetOpen(cur.x + d.x / 2, cur.y + d.y / 2);
                SetOpen(nxt.x, nxt.y);
                stack.push_back(nxt);
                carved = true;
                break;
            }
            if (!carved) stack.pop_back();
        }

        int margin = std::max(3, spec.roomMaxRadius + 1);
        std::uniform_int_distribution<int> pos(margin, std::max(margin, w - margin - 1));
        for (int i = 0; i < spec.roomCount; ++i) {
            int cx = pos(rng) | 1;
            int cy = pos(rng) | 1;
            int span = std::max(1, spec.roomMaxRadius - spec.roomMinRadius + 1);
            int rw = spec.roomMinRadius + static_cast<int>(rng() % span);
            int rh = spec.roomMinRadius + static_cast<int>(rng() % span);
            for (int y = cy - rh; y <= cy + rh; ++y) {
                for (int x = cx - rw; x <= cx + rw; ++x) {
                    if (x > 0 && y > 0 && x < w - 1 && y < h - 1) SetOpen(x, y);
                }
            }
            for (int door = 0; door < 4; ++door) {
                int dx = door < 2 ? (door == 0 ? -rw - 1 : rw + 1) : 0;
                int dy = door >= 2 ? (door == 2 ? -rh - 1 : rh + 1) : 0;
                int px = std::clamp(cx + dx, 1, w - 2);
                int py = std::clamp(cy + dy, 1, h - 2);
                SetOpen(px, py);
            }
        }

        exit = FarthestPerimeterReachable(start);
        AddExtraConnectors(spec);
        exit = FarthestPerimeterReachable(start);
        GenerateWallFeatures(spec);
    }
