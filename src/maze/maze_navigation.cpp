// Maze layout generation and navigation queries.

#include "../platform/platform_headers.h"
#include "maze.h"

Tile Maze::FarthestReachable(Tile from) const {
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

Tile Maze::FarthestPerimeterReachable(Tile from) const {
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

std::vector<int> Maze::ReachableDistances(Tile from) const {
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

bool Maze::LineClear(Tile a, Tile b) const {
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

std::vector<Tile> Maze::Path(Tile from, Tile to) const {
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

int Maze::PathLength(Tile from, Tile to, int minLength) const {
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
