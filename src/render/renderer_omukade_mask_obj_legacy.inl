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
            auto addTri = [&](int ia, int ib, int ic) {
                int ca = clusterIndex(ia);
                int cb = clusterIndex(ib);
                int cc = clusterIndex(ic);
                if (ca == cb || cb == cc || ca == cc) return;
                int sa = ca;
                int sb = cb;
                int sc = cc;
                if (sa > sb) std::swap(sa, sb);
                if (sb > sc) std::swap(sb, sc);
                if (sa > sb) std::swap(sa, sb);
                if (sc >= (1 << 21)) return;
                uint64_t key = static_cast<uint64_t>(sa) |
                    (static_cast<uint64_t>(sb) << 21) |
                    (static_cast<uint64_t>(sc) << 42);
                if (!seen.insert(key).second) return;

                XMFLOAT3 a = localPos(ia);
                XMFLOAT3 b = localPos(ib);
                XMFLOAT3 c = localPos(ic);
                XMFLOAT3 rawNormal = Cross3(Sub3(b, a), Sub3(c, a));
                XMFLOAT3 n = Normalize3(rawNormal, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 faceCenter = Scale3(Add3(Add3(a, b), c), 1.0f / 3.0f);
                if (Dot3(n, faceCenter) < 0.0f) rawNormal = Scale3(rawNormal, -1.0f);
                clusters[static_cast<size_t>(ca)].normal = Add3(clusters[static_cast<size_t>(ca)].normal, rawNormal);
                clusters[static_cast<size_t>(cb)].normal = Add3(clusters[static_cast<size_t>(cb)].normal, rawNormal);
                clusters[static_cast<size_t>(cc)].normal = Add3(clusters[static_cast<size_t>(cc)].normal, rawNormal);
                tris.push_back({ca, cb, cc});
            };

            std::ifstream faces(path);
            if (!faces) return;
            std::string faceLine;
            while (std::getline(faces, faceLine)) {
                if (faceLine.size() < 2 || faceLine[0] != 'f' || faceLine[1] != ' ') continue;
                std::vector<int> poly;
                poly.reserve(8);
                const char* p = faceLine.c_str() + 2;
                while (*p) {
                    int idx = ParseObjFaceIndex(p, static_cast<int>(positions.size()));
                    if (idx >= 0 && idx < static_cast<int>(positions.size())) poly.push_back(idx);
                }
                if (poly.size() < 3) continue;
                for (size_t i = 1; i + 1 < poly.size(); ++i) {
                    addTri(poly[0], poly[i], poly[i + 1]);
                }
            }
        };

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

        out.clear();
        out.reserve(tris.size() * 3);
        auto pushVertex = [&](int cluster, XMFLOAT2 uv) {
            const Cluster& c = clusters[static_cast<size_t>(cluster)];
            XMFLOAT3 p = c.count > 0 ? Scale3(c.sum, 1.0f / static_cast<float>(c.count)) : XMFLOAT3{};
            XMFLOAT3 radial = Normalize3({p.x * 0.86f, p.y * 1.18f, p.z * 0.86f}, Normalize3(p, {0.0f, 1.0f, 0.0f}));
            XMFLOAT3 averaged = Normalize3(c.normal, radial);
            if (Dot3(averaged, radial) < 0.18f) averaged = radial;
            XMFLOAT3 n = Normalize3(Lerp3(averaged, radial, 0.62f), radial);
            XMFLOAT3 tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
            out.push_back({p, n, tangent, uv, 9.65f});
        };
        for (const Tri& tri : tris) {
            pushVertex(tri.a, {0.0f, 0.0f});
            pushVertex(tri.b, {1.0f, 0.0f});
            pushVertex(tri.c, {0.5f, 1.0f});
        }
        return !out.empty();
