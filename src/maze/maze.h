#pragma once

// Maze layout generation and navigation query API.
// Implementation remains include-based in maze.inl for the first extraction slice.

enum class MazeWallFeature : uint8_t {
    None = 0,
    Window = 1,
    Tunnel = 2
};

struct Maze {
    int w = kMazeW;
    int h = kMazeH;
    float tileW = kTile;
    float tileD = kTile;
    std::vector<uint8_t> open;
    std::vector<uint8_t> wallFeatures;
    Tile start{1, 1};
    Tile exit{kMazeW - 2, kMazeH - 2};
    std::mt19937 rng{0xBACC2026u};

    bool InBounds(int x, int y) const;
    bool IsOpen(int x, int y) const;
    void SetOpen(int x, int y, bool v = true);
    MazeWallFeature WallFeature(int x, int y) const;
    bool HasWallFeature(int x, int y) const;
    bool IsVisionOpen(int x, int y) const;
    XMFLOAT3 WorldCenter(Tile t, float y = 0.0f) const;
    Tile TileFromWorld(float x, float z) const;
    float TileAverage() const;
    float TileMinimum() const;
    std::vector<Tile> Neighbors(Tile t) const;

    template <typename Fn>
    void ForEachNeighbor(Tile t, Fn&& fn) const {
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto& d : dirs) {
            Tile n{t.x + d[0], t.y + d[1]};
            if (IsOpen(n.x, n.y)) fn(n);
        }
    }

    int OpenNeighborCount(Tile t) const;
    int LocalOpenCount(Tile t, int radius = 2) const;
    void AddExtraConnectors(const Settings& settings);
    void GenerateWallFeatures(const Settings& settings);
    void Generate(const Settings& settings);
    void GenerateBloodDebugCorridor();
    void GenerateBenchmarkDemo();
    void GenerateDebugSlice(int tiles);
    void GenerateMenuRoom();
    Tile FarthestReachable(Tile from) const;
    Tile FarthestPerimeterReachable(Tile from) const;
    std::vector<int> ReachableDistances(Tile from) const;
    bool LineClear(Tile a, Tile b) const;
    std::vector<Tile> Path(Tile from, Tile to) const;
    int PathLength(Tile from, Tile to, int minLength = 0) const;
};
