    int PathSideBranchCount(Tile cur, Tile previous, Tile nextTarget) const {
        static constexpr int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        int count = 0;
        for (const auto& d : dirs) {
            Tile n{cur.x + d[0], cur.y + d[1]};
            if (!RenderMazeView().IsOpen(n.x, n.y)) continue;
            if (n == previous || n == nextTarget) continue;
            ++count;
        }
        return count;
    }
