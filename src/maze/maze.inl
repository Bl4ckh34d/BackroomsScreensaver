// Maze layout generation and navigation queries.
// Included from main.cpp before renderer code.

struct Maze {
    int w = kMazeW;
    int h = kMazeH;
    float tileW = kTile;
    float tileD = kTile;
    std::vector<uint8_t> open;
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
        if (InBounds(x, y)) open[static_cast<size_t>(y * w + x)] = v ? 1 : 0;
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
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto& d : dirs) {
            Tile n{t.x + d[0], t.y + d[1]};
            if (IsOpen(n.x, n.y)) out.push_back(n);
        }
        return out;
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

    void Generate(const Settings& settings) {
        open.assign(static_cast<size_t>(w * h), 0);
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
    }

    void GenerateBloodDebugCorridor() {
        w = std::max(9, w);
        h = std::max(5, h);
        open.assign(static_cast<size_t>(w * h), 0);
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
        w = 3;
        h = 3;
        open.assign(static_cast<size_t>(w * h), 0);
        start = {1, 1};
        exit = {1, 1};
        SetOpen(1, 1);
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
            for (Tile n : Neighbors(t)) {
                auto idx = static_cast<size_t>(n.y * w + n.x);
                if (dist[idx] >= 0) continue;
                dist[idx] = base + 1;
                q.push(n);
            }
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
            if (!IsOpen(sample.x, sample.y)) return false;
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
            for (Tile n : Neighbors(cur)) {
                int ni = idx(n);
                float next = cost[static_cast<size_t>(idx(cur))] + 1.0f;
                if (next >= cost[static_cast<size_t>(ni)]) continue;
                cost[static_cast<size_t>(ni)] = next;
                parent[static_cast<size_t>(ni)] = idx(cur);
                pq.push({n, next + heuristic(n, to)});
            }
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
};
