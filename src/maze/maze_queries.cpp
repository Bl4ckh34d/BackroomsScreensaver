// Maze layout generation and navigation queries.

#include "../platform/platform_headers.h"
#include "maze.h"

bool Maze::InBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < w && y < h;
    }

bool Maze::IsOpen(int x, int y) const {
        return InBounds(x, y) && open[static_cast<size_t>(y * w + x)] != 0;
    }

void Maze::SetOpen(int x, int y, bool v) {
        if (!InBounds(x, y)) return;
        const size_t idx = static_cast<size_t>(y * w + x);
        open[idx] = v ? 1 : 0;
        if (v && idx < wallFeatures.size()) wallFeatures[idx] = 0;
    }

MazeWallFeature Maze::WallFeature(int x, int y) const {
        if (!InBounds(x, y)) return MazeWallFeature::None;
        const size_t idx = static_cast<size_t>(y * w + x);
        if (idx >= wallFeatures.size()) return MazeWallFeature::None;
        return static_cast<MazeWallFeature>(wallFeatures[idx]);
    }

bool Maze::HasWallFeature(int x, int y) const {
        return WallFeature(x, y) != MazeWallFeature::None;
    }

bool Maze::IsVisionOpen(int x, int y) const {
        return IsOpen(x, y) || WallFeature(x, y) == MazeWallFeature::Window;
    }

XMFLOAT3 Maze::WorldCenter(Tile t, float y) const {
        float ox = -static_cast<float>(w) * tileW * 0.5f;
        float oz = -static_cast<float>(h) * tileD * 0.5f;
        return {ox + (static_cast<float>(t.x) + 0.5f) * tileW, y, oz + (static_cast<float>(t.y) + 0.5f) * tileD};
    }

Tile Maze::TileFromWorld(float x, float z) const {
        float ox = -static_cast<float>(w) * tileW * 0.5f;
        float oz = -static_cast<float>(h) * tileD * 0.5f;
        return {
            static_cast<int>(std::floor((x - ox) / tileW)),
            static_cast<int>(std::floor((z - oz) / tileD))
        };
    }

float Maze::TileAverage() const {
        return (tileW + tileD) * 0.5f;
    }

float Maze::TileMinimum() const {
        return std::min(tileW, tileD);
    }

std::vector<Tile> Maze::Neighbors(Tile t) const {
        std::vector<Tile> out;
        out.reserve(4);
        ForEachNeighbor(t, [&](Tile n) { out.push_back(n); });
        return out;
    }

int Maze::OpenNeighborCount(Tile t) const {
        int count = 0;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto& d : dirs) {
            if (IsOpen(t.x + d[0], t.y + d[1])) ++count;
        }
        return count;
    }

int Maze::LocalOpenCount(Tile t, int radius) const {
        int count = 0;
        for (int y = t.y - radius; y <= t.y + radius; ++y) {
            for (int x = t.x - radius; x <= t.x + radius; ++x) {
                if (IsOpen(x, y)) ++count;
            }
        }
        return count;
    }
