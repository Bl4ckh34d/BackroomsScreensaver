            std::vector<FaceCluster> clusters;
            std::vector<FaceClusterTri> tris;
            int grid = std::clamp(static_cast<int>(std::sqrt(static_cast<float>(maxTriangles) / 2.2f)), 10, 128);
            for (int attempt = 0; attempt < 6; ++attempt) {
                buildCollapsed(grid, clusters, tris);
                if (!tris.empty() && tris.size() <= static_cast<size_t>(maxTriangles)) break;
                grid = std::max(6, static_cast<int>(static_cast<float>(grid) * 0.78f));
            }
            if (clusters.empty() || tris.empty()) return false;
            if (tris.size() > static_cast<size_t>(maxTriangles)) {
                std::vector<FaceClusterTri> reduced;
                reduced.reserve(static_cast<size_t>(maxTriangles));
                double step = static_cast<double>(tris.size()) / static_cast<double>(maxTriangles);
                for (int i = 0; i < maxTriangles; ++i) {
                    reduced.push_back(tris[static_cast<size_t>(std::min<double>(tris.size() - 1, i * step))]);
                }
                tris = std::move(reduced);
            }

            out.clear();
            out.reserve(tris.size() * 3);
            auto pushCluster = [&](int cluster) {
                const FaceCluster& c = clusters[static_cast<size_t>(cluster)];
                float invCount = 1.0f / static_cast<float>(std::max(1, c.count));
                XMFLOAT3 p = Scale3(c.pos, invCount);
                XMFLOAT3 n = Normalize3(c.normal, Normalize3(p, {0.0f, 0.0f, 1.0f}));
                XMFLOAT2 uv{c.uv.x * invCount, c.uv.y * invCount};
                XMFLOAT3 tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
                out.push_back({p, n, tangent, uv, 26.0f});
            };
            for (const FaceClusterTri& tri : tris) {
                pushCluster(tri.a);
                pushCluster(tri.b);
                pushCluster(tri.c);
            }
            return !out.empty();
