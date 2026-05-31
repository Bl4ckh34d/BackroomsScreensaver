// Maze layout generation and navigation queries.
// Included from main.cpp before renderer code.

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

    bool InBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < w && y < h;
    }

    bool IsOpen(int x, int y) const {
        return InBounds(x, y) && open[static_cast<size_t>(y * w + x)] != 0;
    }

    void SetOpen(int x, int y, bool v = true) {
        if (!InBounds(x, y)) return;
        const size_t idx = static_cast<size_t>(y * w + x);
        open[idx] = v ? 1 : 0;
        if (v && idx < wallFeatures.size()) wallFeatures[idx] = 0;
    }

    MazeWallFeature WallFeature(int x, int y) const {
        if (!InBounds(x, y)) return MazeWallFeature::None;
        const size_t idx = static_cast<size_t>(y * w + x);
        if (idx >= wallFeatures.size()) return MazeWallFeature::None;
        return static_cast<MazeWallFeature>(wallFeatures[idx]);
    }

    bool HasWallFeature(int x, int y) const {
        return WallFeature(x, y) != MazeWallFeature::None;
    }

    bool IsVisionOpen(int x, int y) const {
        return IsOpen(x, y) || WallFeature(x, y) == MazeWallFeature::Window;
    }

    XMFLOAT3 WorldCenter(Tile t, float y = 0.0f) const {
        float ox = -static_cast<float>(w) * tileW * 0.5f;
        float oz = -static_cast<float>(h) * tileD * 0.5f;
        return {ox + (static_cast<float>(t.x) + 0.5f) * tileW, y, oz + (static_cast<float>(t.y) + 0.5f) * tileD};
    }

    Tile TileFromWorld(float x, float z) const {
        float ox = -static_cast<float>(w) * tileW * 0.5f;
        float oz = -static_cast<float>(h) * tileD * 0.5f;
        return {
            static_cast<int>(std::floor((x - ox) / tileW)),
            static_cast<int>(std::floor((z - oz) / tileD))
        };
    }

    float TileAverage() const {
        return (tileW + tileD) * 0.5f;
    }

    float TileMinimum() const {
        return std::min(tileW, tileD);
    }

    std::vector<Tile> Neighbors(Tile t) const {
        std::vector<Tile> out;
        out.reserve(4);
        ForEachNeighbor(t, [&](Tile n) { out.push_back(n); });
        return out;
    }

    template <typename Fn>
    void ForEachNeighbor(Tile t, Fn&& fn) const {
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto& d : dirs) {
            Tile n{t.x + d[0], t.y + d[1]};
            if (IsOpen(n.x, n.y)) fn(n);
        }
    }

    int OpenNeighborCount(Tile t) const {
        int count = 0;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto& d : dirs) {
            if (IsOpen(t.x + d[0], t.y + d[1])) ++count;
        }
        return count;
    }

    int LocalOpenCount(Tile t, int radius = 2) const {
        int count = 0;
        for (int y = t.y - radius; y <= t.y + radius; ++y) {
            for (int x = t.x - radius; x <= t.x + radius; ++x) {
                if (IsOpen(x, y)) ++count;
            }
        }
        return count;
    }

    void AddExtraConnectors(const Settings& settings) {
        float minRatio = std::clamp(settings.extraConnectorMinRatio, 0.015f, 0.20f);
        float maxRatio = std::clamp(settings.extraConnectorMaxRatio, minRatio, 0.20f);
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

    void GenerateWallFeatures(const Settings& settings) {
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

        const float baseFrequency = std::clamp(static_cast<float>(settings.wallFeatureFrequency), 1.0f, 200.0f);
        const float spread = std::clamp(settings.wallFeatureFrequencySpread, 0.0f, 3.0f);
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

    void Generate(const Settings& settings) {
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

        int margin = std::max(3, settings.roomMaxRadius + 1);
        std::uniform_int_distribution<int> pos(margin, std::max(margin, w - margin - 1));
        for (int i = 0; i < settings.roomCount; ++i) {
            int cx = pos(rng) | 1;
            int cy = pos(rng) | 1;
            int span = std::max(1, settings.roomMaxRadius - settings.roomMinRadius + 1);
            int rw = settings.roomMinRadius + static_cast<int>(rng() % span);
            int rh = settings.roomMinRadius + static_cast<int>(rng() % span);
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
        AddExtraConnectors(settings);
        exit = FarthestPerimeterReachable(start);
        GenerateWallFeatures(settings);
    }

    void GenerateBloodDebugCorridor() {
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

    void GenerateDebugSlice(int tiles) {
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

    void GenerateMenuRoom() {
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

    Tile FarthestReachable(Tile from) const {
        std::vector<int> dist = ReachableDistances(from);
        Tile best = from;
        auto idx = [this](Tile t) { return static_cast<size_t>(t.y * w + t.x); };
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                Tile t{x, y};
                if (dist[idx(t)] > dist[idx(best)]) best = t;
            }
        }
        return best;
    }

    Tile FarthestPerimeterReachable(Tile from) const {
        std::vector<int> dist = ReachableDistances(from);
        Tile best = from;
        int bestDist = -1;
        auto idx = [this](Tile t) { return static_cast<size_t>(t.y * w + t.x); };
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (!IsOpen(x, y)) continue;
                bool perimeterPortal = (x == 1) || (x == w - 2) || (y == 1) || (y == h - 2);
                if (!perimeterPortal) continue;
                int d = dist[idx({x, y})];
                if (d > bestDist) {
                    bestDist = d;
                    best = {x, y};
                }
            }
        }
        return bestDist >= 0 ? best : FarthestReachable(from);
    }

    std::vector<int> ReachableDistances(Tile from) const {
        std::vector<int> dist(static_cast<size_t>(w * h), -1);
        std::queue<Tile> q;
        q.push(from);
        dist[static_cast<size_t>(from.y * w + from.x)] = 0;
        while (!q.empty()) {
            Tile t = q.front();
            q.pop();
            int base = dist[static_cast<size_t>(t.y * w + t.x)];
            ForEachNeighbor(t, [&](Tile n) {
                auto idx = static_cast<size_t>(n.y * w + n.x);
                if (dist[idx] >= 0) return;
                dist[idx] = base + 1;
                q.push(n);
            });
        }
        return dist;
    }

    bool LineClear(Tile a, Tile b) const {
        XMFLOAT3 aw = WorldCenter(a, 1.5f);
        XMFLOAT3 bw = WorldCenter(b, 1.5f);
        float dx = bw.x - aw.x;
        float dz = bw.z - aw.z;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(2, static_cast<int>(len / (TileMinimum() * 0.18f)));
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            Tile sample = TileFromWorld(aw.x + dx * t, aw.z + dz * t);
            if (!IsVisionOpen(sample.x, sample.y)) return false;
        }
        return true;
    }

    std::vector<Tile> Path(Tile from, Tile to) const {
        if (!IsOpen(from.x, from.y) || !IsOpen(to.x, to.y)) return {};
        const int count = w * h;
        std::vector<float> cost(static_cast<size_t>(count), std::numeric_limits<float>::infinity());
        std::vector<int> parent(static_cast<size_t>(count), -1);
        auto idx = [this](Tile t) { return t.y * w + t.x; };
        auto heuristic = [](Tile a, Tile b) {
            return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
        };
        struct Node {
            Tile t;
            float f;
            bool operator<(const Node& other) const { return f > other.f; }
        };
        std::priority_queue<Node> pq;
        cost[static_cast<size_t>(idx(from))] = 0.0f;
        pq.push({from, heuristic(from, to)});
        while (!pq.empty()) {
            Tile cur = pq.top().t;
            pq.pop();
            if (cur == to) break;
            ForEachNeighbor(cur, [&](Tile n) {
                int ni = idx(n);
                float next = cost[static_cast<size_t>(idx(cur))] + 1.0f;
                if (next >= cost[static_cast<size_t>(ni)]) return;
                cost[static_cast<size_t>(ni)] = next;
                parent[static_cast<size_t>(ni)] = idx(cur);
                pq.push({n, next + heuristic(n, to)});
            });
        }
        if (!std::isfinite(cost[static_cast<size_t>(idx(to))])) return {};
        std::vector<Tile> out;
        int at = idx(to);
        while (at != -1) {
            out.push_back({at % w, at / w});
            if (at == idx(from)) break;
            at = parent[static_cast<size_t>(at)];
        }
        std::reverse(out.begin(), out.end());
        return out;
    }

    int PathLength(Tile from, Tile to, int minLength = 0) const {
        if (!IsOpen(from.x, from.y) || !IsOpen(to.x, to.y)) return 0;
        const int count = w * h;
        std::vector<int> dist(static_cast<size_t>(count), -1);
        auto idx = [this](Tile t) { return t.y * w + t.x; };
        std::queue<Tile> q;
        int fromIndex = idx(from);
        dist[static_cast<size_t>(fromIndex)] = 1;
        q.push(from);
        while (!q.empty()) {
            Tile cur = q.front();
            q.pop();
            int curDist = dist[static_cast<size_t>(idx(cur))];
            if (cur == to) return curDist;
            ForEachNeighbor(cur, [&](Tile n) {
                int ni = idx(n);
                if (dist[static_cast<size_t>(ni)] >= 0) return;
                dist[static_cast<size_t>(ni)] = curDist + 1;
                q.push(n);
            });
        }
        (void)minLength;
        return 0;
    }
};
