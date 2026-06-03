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
