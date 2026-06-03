
        if (!RenderMazeView().IsOpen(cur.x, cur.y)) return {};
        int count = RenderMazeView().w * RenderMazeView().h;
        auto idx = [this](Tile t) { return t.y * RenderMazeView().w + t.x; };
        int startIndex = idx(cur);
        std::vector<int> parent(static_cast<size_t>(count), -1);
        std::vector<int> depth(static_cast<size_t>(count), -1);
        std::queue<Tile> q;
        parent[static_cast<size_t>(startIndex)] = startIndex;
        depth[static_cast<size_t>(startIndex)] = 0;
        q.push(cur);
        while (!q.empty()) {
            Tile t = q.front();
            q.pop();
            RenderMazeView().ForEachNeighbor(t, [&](Tile n) {
                int ni = idx(n);
                if (depth[static_cast<size_t>(ni)] >= 0) return;
                parent[static_cast<size_t>(ni)] = idx(t);
                depth[static_cast<size_t>(ni)] = depth[static_cast<size_t>(idx(t))] + 1;
                q.push(n);
            });
        }
