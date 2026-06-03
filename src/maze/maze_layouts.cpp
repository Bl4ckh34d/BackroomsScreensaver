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

        openRect(8, 14, 66, 60);
        openRect(2, 34, 72, 40);
        openRect(34, 2, 40, 72);
        openRect(4, 8, 12, 24);
        openRect(58, 50, 70, 68);

        closeRect(22, 24, 25, 33);
        closeRect(48, 22, 52, 31);
        closeRect(31, 46, 37, 51);
        closeRect(53, 42, 58, 47);
        closeRect(15, 44, 19, 52);

        openRect(24, 29, 31, 30);
        openRect(36, 48, 46, 49);
        openRect(51, 44, 54, 44);

        start = {13, 54};
        exit = {61, 20};
        SetOpen(start.x, start.y);
        SetOpen(exit.x, exit.y);
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
