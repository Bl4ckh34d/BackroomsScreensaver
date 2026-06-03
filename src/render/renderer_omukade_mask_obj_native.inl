        if (nativeMaskAxes) {
            struct FaceV {
                int p;
                int t;
                int n;
            };
            std::vector<std::array<FaceV, 3>> faceTris;
            faceTris.reserve(80000);
            std::ifstream faces(path);
            if (!faces) return false;
            std::string faceLine;
            while (std::getline(faces, faceLine)) {
                if (faceLine.size() < 2 || faceLine[0] != 'f' || faceLine[1] != ' ') continue;
                std::vector<ObjFaceVertex> poly;
                poly.reserve(8);
                const char* p = faceLine.c_str() + 2;
                while (*p) {
                    ObjFaceVertex v = ParseObjFaceVertex(p, static_cast<int>(positions.size()),
                        static_cast<int>(texcoords.size()), static_cast<int>(normals.size()));
                    if (v.vertex >= 0 && v.vertex < static_cast<int>(positions.size())) poly.push_back(v);
                }
                if (poly.size() < 3) continue;
                for (size_t i = 1; i + 1 < poly.size(); ++i) {
                    faceTris.push_back({FaceV{poly[0].vertex, poly[0].texcoord, poly[0].normal},
                        FaceV{poly[i].vertex, poly[i].texcoord, poly[i].normal},
                        FaceV{poly[i + 1].vertex, poly[i + 1].texcoord, poly[i + 1].normal}});
                }
            }
            if (faceTris.empty()) return false;

            auto pushFaceVertex = [&](const FaceV& fv) {
                XMFLOAT3 p = localPos(fv.p);
                XMFLOAT3 n = (fv.n >= 0 && fv.n < static_cast<int>(normals.size()))
                    ? localNormal(fv.n)
                    : Normalize3(p, {0.0f, 0.0f, 1.0f});
                XMFLOAT2 uv = (fv.t >= 0 && fv.t < static_cast<int>(texcoords.size()))
                    ? texcoords[static_cast<size_t>(fv.t)]
                    : XMFLOAT2{0.5f + p.x * 0.5f, 0.5f - p.y * 0.5f};
                XMFLOAT3 tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
                out.push_back({p, n, tangent, uv, 26.0f});
            };
            if (faceTris.size() <= static_cast<size_t>(maxTriangles)) {
                out.clear();
                out.reserve(faceTris.size() * 3);
                for (const auto& tri : faceTris) {
                    pushFaceVertex(tri[0]);
                    pushFaceVertex(tri[1]);
                    pushFaceVertex(tri[2]);
                }
                return !out.empty();
            }

            struct FaceCluster {
                XMFLOAT3 pos{0.0f, 0.0f, 0.0f};
                XMFLOAT3 normal{0.0f, 0.0f, 0.0f};
                XMFLOAT2 uv{0.0f, 0.0f};
                int count = 0;
            };
            struct FaceClusterTri {
                int a;
                int b;
                int c;
            };

            auto buildCollapsed = [&](int grid, std::vector<FaceCluster>& clusters, std::vector<FaceClusterTri>& tris) {
                clusters.clear();
                tris.clear();
                std::unordered_map<int, int> clusterByCell;
                std::unordered_set<uint64_t> seen;
                clusterByCell.reserve(static_cast<size_t>(grid * grid * 4));
                seen.reserve(static_cast<size_t>(maxTriangles * 2));
                auto clusterIndex = [&](const FaceV& fv) {
                    XMFLOAT3 p = localPos(fv.p);
                    XMFLOAT3 n = (fv.n >= 0 && fv.n < static_cast<int>(normals.size()))
                        ? localNormal(fv.n)
                        : Normalize3(p, {0.0f, 0.0f, 1.0f});
                    XMFLOAT2 uv = (fv.t >= 0 && fv.t < static_cast<int>(texcoords.size()))
                        ? texcoords[static_cast<size_t>(fv.t)]
                        : XMFLOAT2{0.5f + p.x * 0.5f, 0.5f - p.y * 0.5f};
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
                    FaceCluster& c = clusters[static_cast<size_t>(ci)];
                    c.pos = Add3(c.pos, p);
                    c.normal = Add3(c.normal, n);
                    c.uv.x += uv.x;
                    c.uv.y += uv.y;
                    ++c.count;
                    return ci;
                };
                for (const auto& tri : faceTris) {
                    int ca = clusterIndex(tri[0]);
                    int cb = clusterIndex(tri[1]);
                    int cc = clusterIndex(tri[2]);
                    if (ca == cb || cb == cc || ca == cc) continue;
                    int sa = ca;
                    int sb = cb;
                    int sc = cc;
                    if (sa > sb) std::swap(sa, sb);
                    if (sb > sc) std::swap(sb, sc);
                    if (sa > sb) std::swap(sa, sb);
                    if (sc >= (1 << 21)) continue;
                    uint64_t key = static_cast<uint64_t>(sa) |
                        (static_cast<uint64_t>(sb) << 21) |
                        (static_cast<uint64_t>(sc) << 42);
                    if (!seen.insert(key).second) continue;
                    tris.push_back({ca, cb, cc});
                }
            };

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
        }
