        auto pathTo = [&](Tile target) {
            std::vector<Tile> out;
            int at = idx(target);
            if (at < 0 || at >= count || depth[static_cast<size_t>(at)] < 0) return out;
            while (at != -1) {
                out.push_back({at % RenderMazeView().w, at / RenderMazeView().w});
                if (at == startIndex) break;
                at = parent[static_cast<size_t>(at)];
            }
            std::reverse(out.begin(), out.end());
            return out;
        };
