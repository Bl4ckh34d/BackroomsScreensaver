    int FirstBranchIndex(const std::vector<Tile>& path, int limit = 9999) const {
        int count = std::min<int>(static_cast<int>(path.size()), limit + 1);
        for (int i = 1; i < count; ++i) {
            Tile t = path[static_cast<size_t>(i)];
            if (RenderMazeView().OpenNeighborCount(t) >= 3 || RenderMazeView().LocalOpenCount(t, 2) >= 13) return i;
        }
        return -1;
    }
