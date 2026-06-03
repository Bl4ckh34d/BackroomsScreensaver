        auto buildClustered = [&](int grid, std::vector<Cluster>& clusters, std::vector<Tri>& tris) {
            clusters.clear();
            tris.clear();
            std::unordered_map<int, int> clusterByCell;
            std::unordered_set<uint64_t> seen;
            clusterByCell.reserve(static_cast<size_t>(grid * grid * 4));
            seen.reserve(static_cast<size_t>(maxTriangles * 2));
            auto clusterIndex = [&](int idx) {
                XMFLOAT3 p = localPos(idx);
                int ix = std::clamp(static_cast<int>((p.x / 1.12f + 0.5f) * static_cast<float>(grid)), 0, grid - 1);
                int iy = std::clamp(static_cast<int>((p.y / 1.12f + 0.5f) * static_cast<float>(grid)), 0, grid - 1);
                int iz = std::clamp(static_cast<int>((p.z / 1.12f + 0.5f) * static_cast<float>(grid)), 0, grid - 1);
                int key = ix + iy * grid + iz * grid * grid;
                auto found = clusterByCell.find(key);
                int ci = 0;
                if (found == clusterByCell.end()) {
                    ci = static_cast<int>(clusters.size());
                    clusterByCell.emplace(key, ci);
                    clusters.push_back({});
                } else {
                    ci = found->second;
                }
                Cluster& c = clusters[static_cast<size_t>(ci)];
                c.sum = Add3(c.sum, p);
                ++c.count;
                return ci;
            };
