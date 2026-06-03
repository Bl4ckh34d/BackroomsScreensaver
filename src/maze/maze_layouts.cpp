// Maze layout generation and navigation queries.

#include "../platform/platform_headers.h"
#include "maze.h"

void Maze::GenerateBloodDebugCorridor() {
        w = std::max(9, w);
        h = std::max(5, h);
        open.assign(static_cast<size_t>(w * h), 0);
        wallFeatures.assign(static_cast<size_t>(w * h), 0);
        int row = std::clamp(h / 2, 1, h - 2);
        for (int x = 1; x < w - 1; ++x) {
            SetOpen(x, row);
        }
        start = {1, row};
        exit = {w - 2, row};
    }

void Maze::GenerateBenchmarkDemo() {
        w = 75;
        h = 75;
        open.assign(static_cast<size_t>(w * h), 0);
        wallFeatures.assign(static_cast<size_t>(w * h), 0);

        auto openRect = [&](int x0, int y0, int x1, int y1) {
            for (int y = std::max(1, y0); y <= std::min(h - 2, y1); ++y) {
                for (int x = std::max(1, x0); x <= std::min(w - 2, x1); ++x) {
                    SetOpen(x, y);
                }
            }
        };
        auto closeRect = [&](int x0, int y0, int x1, int y1) {
            for (int y = std::max(1, y0); y <= std::min(h - 2, y1); ++y) {
                for (int x = std::max(1, x0); x <= std::min(w - 2, x1); ++x) {
                    SetOpen(x, y, false);
                }
            }
        };
        auto openDisc = [&](int cx, int cy, int radius) {
            for (int y = cy - radius; y <= cy + radius; ++y) {
                for (int x = cx - radius; x <= cx + radius; ++x) {
                    if (std::abs(x - cx) + std::abs(y - cy) <= radius) SetOpen(x, y);
                }
            }
        };
        auto openPath = [&](std::initializer_list<Tile> points, int radius) {
            if (points.size() < 2) return;
            auto it = points.begin();
            Tile prev = *it++;
            openDisc(prev.x, prev.y, radius);
            for (; it != points.end(); ++it) {
                Tile next = *it;
                Tile cur = prev;
                while (cur.x != next.x || cur.y != next.y) {
                    if (cur.x != next.x) cur.x += (next.x > cur.x) ? 1 : -1;
                    if (cur.y != next.y) cur.y += (next.y > cur.y) ? 1 : -1;
                    openDisc(cur.x, cur.y, radius);
                }
                prev = next;
            }
        };

        openRect(7, 48, 20, 62);
        openRect(8, 8, 23, 20);
        openRect(28, 29, 45, 45);
        openRect(55, 15, 69, 29);
        openRect(52, 52, 67, 66);

        openPath({{13, 55}, {13, 38}, {24, 38}, {24, 31}, {35, 31}, {35, 17}, {56, 17}, {56, 23}, {64, 23}}, 1);
        openPath({{35, 38}, {57, 38}, {57, 58}, {62, 58}}, 1);
        openPath({{16, 20}, {16, 30}, {24, 30}, {36, 30}}, 1);
        openPath({{41, 44}, {41, 54}, {53, 54}}, 1);
        openPath({{20, 55}, {31, 55}, {31, 44}}, 1);
        openPath({{45, 36}, {54, 36}, {54, 23}}, 1);

        closeRect(34, 33, 38, 39);
        closeRect(29, 37, 31, 43);
        closeRect(59, 19, 62, 25);
        closeRect(12, 12, 15, 16);
        closeRect(58, 56, 62, 60);

        openPath({{31, 36}, {36, 36}, {41, 40}}, 1);
        openPath({{57, 23}, {64, 23}}, 1);
        openPath({{13, 17}, {19, 17}}, 1);
        openPath({{56, 58}, {65, 58}}, 1);

        start = {13, 55};
        exit = {64, 23};
        SetOpen(start.x, start.y);
        SetOpen(exit.x, exit.y);
        MazeGenerationSpec features{};
        features.wallFeatureFrequency = 18;
        features.wallFeatureFrequencySpread = 0.0f;
        GenerateWallFeatures(features);
    }

void Maze::GenerateDebugSlice(int tiles) {
        tiles = std::clamp(tiles, 1, 5);
        w = tiles + 2;
        h = tiles + 2;
        open.assign(static_cast<size_t>(w * h), 0);
        wallFeatures.assign(static_cast<size_t>(w * h), 0);
        for (int y = 1; y <= tiles; ++y) {
            for (int x = 1; x <= tiles; ++x) {
                SetOpen(x, y);
            }
        }
        int mid = 1 + tiles / 2;
        start = {mid, tiles};
        exit = {mid, 1};
    }

void Maze::GenerateMenuRoom() {
        w = 14;
        h = 24;
        open.assign(static_cast<size_t>(w * h), 0);
        wallFeatures.assign(static_cast<size_t>(w * h), 0);
        start = {9, 21};
        exit = start;
        for (int y = start.y - 2; y <= start.y; ++y) {
            for (int x = start.x; x <= start.x + 2; ++x) {
                SetOpen(x, y);
            }
        }
        const int hallX = std::clamp(start.x + 1, 1, w - 2);
        for (int y = 1; y <= start.y - 2; ++y) {
            SetOpen(hallX, y);
        }
    }
