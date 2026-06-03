        std::vector<Cluster> clusters;
        std::vector<Tri> tris;
        int grid = std::clamp(static_cast<int>(std::sqrt(static_cast<float>(maxTriangles) / 3.0f)), 18, 128);
        for (int attempt = 0; attempt < 4; ++attempt) {
            buildClustered(grid, clusters, tris);
            if (!tris.empty() && tris.size() <= static_cast<size_t>(maxTriangles)) break;
            grid = std::max(10, static_cast<int>(static_cast<float>(grid) * 0.82f));
        }
        if (clusters.empty() || tris.empty()) return false;
        if (tris.size() > static_cast<size_t>(maxTriangles)) {
            std::vector<Tri> reduced;
            reduced.reserve(static_cast<size_t>(maxTriangles));
            double step = static_cast<double>(tris.size()) / static_cast<double>(maxTriangles);
            for (int i = 0; i < maxTriangles; ++i) {
                reduced.push_back(tris[static_cast<size_t>(std::min<double>(tris.size() - 1, i * step))]);
            }
            tris = std::move(reduced);
        }
