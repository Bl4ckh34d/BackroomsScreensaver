// Maze layout generation and navigation queries.

#include "../platform/platform_headers.h"
#include "maze.h"

void Maze::AddExtraConnectors(const MazeGenerationSpec& spec) {
        float minRatio = std::clamp(spec.extraConnectorMinRatio, 0.015f, 0.20f);
        float maxRatio = std::clamp(spec.extraConnectorMaxRatio, minRatio, 0.20f);
        if (maxRatio <= 0.0f) return;

        std::vector<Tile> candidates;
        candidates.reserve(static_cast<size_t>(w * h / 4));
        for (int y = 1; y < h - 1; ++y) {
            for (int x = 1; x < w - 1; ++x) {
                if (IsOpen(x, y)) continue;
                Tile t{x, y};
                if (std::abs(x - start.x) + std::abs(y - start.y) <= 2 ||
                    std::abs(x - exit.x) + std::abs(y - exit.y) <= 2) {
                    continue;
                }
                bool east = IsOpen(x + 1, y);
                bool west = IsOpen(x - 1, y);
                bool south = IsOpen(x, y + 1);
                bool north = IsOpen(x, y - 1);
                int openSides = (east ? 1 : 0) + (west ? 1 : 0) + (south ? 1 : 0) + (north ? 1 : 0);
                if (openSides < 2) continue;
                bool usefulConnector = (east && west) || (north && south) || openSides >= 3 ||
                    ((east || west) && (north || south));
                if (!usefulConnector) continue;
                candidates.push_back(t);
            }
        }
        if (candidates.empty()) return;

        std::uniform_real_distribution<float> ratioDist(minRatio, maxRatio);
        float ratio = ratioDist(rng);
        int target = std::clamp(static_cast<int>(std::round(static_cast<float>(candidates.size()) * ratio)),
            ratio > 0.0f ? 1 : 0, static_cast<int>(candidates.size()));
        std::shuffle(candidates.begin(), candidates.end(), rng);
        for (int i = 0; i < target; ++i) {
            SetOpen(candidates[static_cast<size_t>(i)].x, candidates[static_cast<size_t>(i)].y);
        }
    }
