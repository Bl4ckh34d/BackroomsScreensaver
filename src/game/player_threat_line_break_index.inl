    int FirstThreatLineBreakIndex(const std::vector<Tile>& path, Tile monsterTile, int limit = 9999) const {
        int count = std::min<int>(static_cast<int>(path.size()), limit + 1);
        for (int i = 1; i < count; ++i) {
            if (!RenderMazeView().LineClear(path[static_cast<size_t>(i)], monsterTile)) return i;
        }
        return -1;
    }
