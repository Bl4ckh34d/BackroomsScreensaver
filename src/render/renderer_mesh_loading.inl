// Renderer asset path resolution, monster mesh loading, and static prop mesh loading.
// Included inside Renderer private section before shader/resource creation helpers.

    std::filesystem::path ResolveConfiguredAssetPath(const std::wstring& configuredPath) const {
        if (configuredPath.empty()) return {};
        std::filesystem::path p(configuredPath);
        std::vector<std::filesystem::path> candidates;
        auto add = [&](std::filesystem::path c) {
            c = c.lexically_normal();
            if (std::find(candidates.begin(), candidates.end(), c) == candidates.end()) candidates.push_back(std::move(c));
        };
        if (p.is_absolute()) {
            add(p);
        } else {
            std::filesystem::path module = ModuleDirectory();
            add(module / p);
            add(module.parent_path() / p);
            add(module.parent_path().parent_path() / p);
            add(std::filesystem::current_path() / p);
            std::filesystem::path resolved = ResolveAsset(settings_, configuredPath);
            if (!resolved.empty()) add(resolved);
        }
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec)) return candidate;
        }
        return {};
    }

    static bool IsNativeMaskMeshPath(const std::filesystem::path& path) {
        std::wstring lowered = path.wstring();
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        return lowered.find(L"monster_face_mask") != std::wstring::npos ||
            lowered.find(L"horror_mask") != std::wstring::npos;
    }

    uint64_t MonsterMeshCacheHash(const std::filesystem::path& path) const {
        uint64_t hash = 1469598103934665603ull;
        const char* version = "BackroomsMazeMonsterMeshCacheV11";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, &settings_.monsterSkullMaxTriangles, sizeof(settings_.monsterSkullMaxTriangles));
        bool nativeMaskAxes = IsNativeMaskMeshPath(path);
        hash = Fnv1aAppend(hash, &nativeMaskAxes, sizeof(nativeMaskAxes));
        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        hash = HashWide(hash, ext);
        return HashFileSignature(hash, path);
    }

    static std::wstring MonsterMeshCacheFileName(uint64_t hash) {
        std::wstringstream name;
        name << L"BackroomsMaze_skullmesh_" << std::hex << hash << L".bin";
        return name.str();
    }

    static std::filesystem::path MonsterMeshCachePath(uint64_t hash) {
        return CacheDirectory() / MonsterMeshCacheFileName(hash);
    }

    static bool LoadMonsterMeshCache(uint64_t hash, std::vector<Vertex>& out) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t vertexCount;
            uint32_t reserved;
        };
        std::filesystem::path bundledPath = ModuleDirectory() / L"MeshCache" / MonsterMeshCacheFileName(hash);
        std::filesystem::path writablePath = MonsterMeshCachePath(hash);
        const std::array<std::filesystem::path, 2> candidates = {writablePath, bundledPath};
        for (const auto& candidate : candidates) {
            Header header{};
            std::ifstream in(candidate, std::ios::binary);
            if (!in) continue;
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (!in || std::memcmp(header.magic, "BRMMSH1", 7) != 0 || header.hash != hash ||
                header.vertexCount == 0 || header.vertexCount > static_cast<uint32_t>(kDynamicVertexCapacity)) {
                continue;
            }
            out.resize(header.vertexCount);
            in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size() * sizeof(Vertex)));
            if (in.good()) return true;
            out.clear();
        }
        return false;
    }

    static bool LoadMonsterMeshFile(const std::filesystem::path& path, std::vector<Vertex>& out) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t vertexCount;
            uint32_t reserved;
        };
        Header header{};
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!in || std::memcmp(header.magic, "BRMMSH1", 7) != 0 ||
            header.vertexCount == 0 || header.vertexCount > static_cast<uint32_t>(kDynamicVertexCapacity)) {
            return false;
        }
        out.resize(header.vertexCount);
        in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size() * sizeof(Vertex)));
        if (in.good()) return true;
        out.clear();
        return false;
    }

    static void SaveMonsterMeshCache(uint64_t hash, const std::vector<Vertex>& mesh) {
        if (mesh.empty() || mesh.size() > static_cast<size_t>(kDynamicVertexCapacity)) return;
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t vertexCount;
            uint32_t reserved;
        };
        Header header{{'B', 'R', 'M', 'M', 'S', 'H', '1', '\0'}, hash, static_cast<uint32_t>(mesh.size()), 0};
        std::ofstream out(MonsterMeshCachePath(hash), std::ios::binary | std::ios::trunc);
        if (!out) return;
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(mesh.data()), static_cast<std::streamsize>(mesh.size() * sizeof(Vertex)));
    }

    static int ParseObjFaceIndex(const char*& p, int vertexCount) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) return std::numeric_limits<int>::min();
        char* end = nullptr;
        long raw = std::strtol(p, &end, 10);
        if (end == p) {
            while (*p && !std::isspace(static_cast<unsigned char>(*p))) ++p;
            return std::numeric_limits<int>::min();
        }
        while (*end && !std::isspace(static_cast<unsigned char>(*end))) ++end;
        p = end;
        if (raw > 0) return static_cast<int>(raw - 1);
        if (raw < 0) return vertexCount + static_cast<int>(raw);
        return std::numeric_limits<int>::min();
    }

    struct ObjFaceVertex {
        int vertex = std::numeric_limits<int>::min();
        int texcoord = std::numeric_limits<int>::min();
        int normal = std::numeric_limits<int>::min();
    };

    static int ResolveObjIndex(long raw, int count) {
        if (raw > 0) return static_cast<int>(raw - 1);
        if (raw < 0) return count + static_cast<int>(raw);
        return std::numeric_limits<int>::min();
    }

    static ObjFaceVertex ParseObjFaceVertex(const char*& p, int vertexCount, int texcoordCount, int normalCount = 0) {
        ObjFaceVertex result{};
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) return result;

        char* end = nullptr;
        long rawVertex = std::strtol(p, &end, 10);
        if (end == p) {
            while (*p && !std::isspace(static_cast<unsigned char>(*p))) ++p;
            return result;
        }
        result.vertex = ResolveObjIndex(rawVertex, vertexCount);

        const char* q = end;
        if (*q == '/') {
            ++q;
            if (*q && *q != '/') {
                char* uvEnd = nullptr;
                long rawUv = std::strtol(q, &uvEnd, 10);
                if (uvEnd != q) {
                    result.texcoord = ResolveObjIndex(rawUv, texcoordCount);
                    q = uvEnd;
                }
            }
            if (*q == '/') {
                ++q;
                if (*q) {
                    char* normalEnd = nullptr;
                    long rawNormal = std::strtol(q, &normalEnd, 10);
                    if (normalEnd != q) {
                        result.normal = ResolveObjIndex(rawNormal, normalCount);
                        q = normalEnd;
                    }
                }
            }
        }
        while (*q && !std::isspace(static_cast<unsigned char>(*q))) ++q;
        p = q;
        return result;
    }

    bool LoadMonsterSkullObj(const std::filesystem::path& path, std::vector<Vertex>& out) const {
        struct Cluster {
            XMFLOAT3 sum{0.0f, 0.0f, 0.0f};
            XMFLOAT3 normal{0.0f, 0.0f, 0.0f};
            int count = 0;
        };
        struct Tri {
            int a;
            int b;
            int c;
        };

        int maxTriangles = std::max(0, settings_.monsterSkullMaxTriangles);
        if (maxTriangles <= 0) return false;

        std::ifstream in(path);
        if (!in) return false;

        std::vector<XMFLOAT3> positions;
        std::vector<XMFLOAT2> texcoords;
        std::vector<XMFLOAT3> normals;
        positions.reserve(250000);
        XMFLOAT3 minP{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT3 maxP{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        std::string line;
        while (std::getline(in, line)) {
            if (line.size() >= 2 && line[0] == 'v' && line[1] == ' ') {
                const char* p = line.c_str() + 2;
                char* end = nullptr;
                float x = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float y = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float z = std::strtof(p, &end);
                if (end == p) continue;
                positions.push_back({x, y, z});
                minP.x = std::min(minP.x, x);
                minP.y = std::min(minP.y, y);
                minP.z = std::min(minP.z, z);
                maxP.x = std::max(maxP.x, x);
                maxP.y = std::max(maxP.y, y);
                maxP.z = std::max(maxP.z, z);
            } else if (line.size() >= 3 && line[0] == 'v' && line[1] == 't' && std::isspace(static_cast<unsigned char>(line[2]))) {
                const char* p = line.c_str() + 3;
                char* end = nullptr;
                float u = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float v = std::strtof(p, &end);
                if (end == p) continue;
                texcoords.push_back({u, 1.0f - v});
            } else if (line.size() >= 3 && line[0] == 'v' && line[1] == 'n' && std::isspace(static_cast<unsigned char>(line[2]))) {
                const char* p = line.c_str() + 3;
                char* end = nullptr;
                float x = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float y = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float z = std::strtof(p, &end);
                if (end == p) continue;
                normals.push_back({x, y, z});
            }
        }
        if (positions.empty()) return false;

        XMFLOAT3 center{(minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f, (minP.z + maxP.z) * 0.5f};
        float maxDim = std::max({maxP.x - minP.x, maxP.y - minP.y, maxP.z - minP.z, 0.001f});
        float scale = 1.12f / maxDim;
        bool nativeMaskAxes = IsNativeMaskMeshPath(path);
        auto localPos = [&](int idx) {
            const XMFLOAT3& p = positions[static_cast<size_t>(idx)];
            if (nativeMaskAxes) {
                return XMFLOAT3{(p.x - center.x) * scale, (p.y - center.y) * scale, (p.z - center.z) * scale};
            }
            return XMFLOAT3{(p.x - center.x) * scale, (p.z - center.z) * scale, (p.y - center.y) * scale};
        };
        auto localNormal = [&](int idx) {
            const XMFLOAT3& n = normals[static_cast<size_t>(idx)];
            if (nativeMaskAxes) return Normalize3(n, {0.0f, 0.0f, 1.0f});
            return Normalize3({n.x, n.z, n.y}, {0.0f, 0.0f, 1.0f});
        };

        if (nativeMaskAxes && !texcoords.empty()) {
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
    }

    bool LoadMonsterSkullMesh() {
        skullMesh_.clear();
        monsterMeshLoaded_ = false;
        monsterUsingAltSkull_ = false;
        monsterSkullNativeMaskAxes_ = false;
        std::wstring configuredPath = settings_.monsterSkullMesh;
        if (!settings_.monsterAltSkullMesh.empty() &&
            Rand01(913, 917, runtimeSeed_) < std::clamp(settings_.monsterAltSkullChance, 0.0f, 1.0f)) {
            configuredPath = settings_.monsterAltSkullMesh;
            monsterUsingAltSkull_ = true;
        }
        std::filesystem::path path = ResolveConfiguredAssetPath(configuredPath);
        if (path.empty()) {
            StartupProfileLine(L"Monster skull mesh not found: " + configuredPath);
            return false;
        }
        monsterSkullNativeMaskAxes_ = IsNativeMaskMeshPath(path);
        uint64_t hash = MonsterMeshCacheHash(path);
        if (LoadMonsterMeshCache(hash, skullMesh_)) {
            StartupProfileLine(L"Loaded cached monster skull mesh: " + std::to_wstring(skullMesh_.size() / 3) + L" tris");
            monsterMeshLoaded_ = true;
            return true;
        }

        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        bool ok = false;
        if (ext == L".bin") {
            ok = LoadMonsterMeshFile(path, skullMesh_);
        } else if (ext == L".obj") {
            ok = LoadMonsterSkullObj(path, skullMesh_);
        }
        if (ok) {
            monsterMeshLoaded_ = true;
            if (ext != L".bin") {
                SaveMonsterMeshCache(hash, skullMesh_);
            }
            StartupProfileLine(L"Loaded monster skull mesh: " + path.wstring() + L" (" + std::to_wstring(skullMesh_.size() / 3) + L" tris)");
        } else {
            skullMesh_.clear();
            StartupProfileLine(L"Could not load monster skull mesh: " + path.wstring());
        }
        return ok;
    }

    static bool LoadStaticPropBinary(const std::filesystem::path& path, StaticPropMesh& out) {
        struct Header {
            char magic[8];
            uint32_t vertexCount;
            uint32_t vertexStride;
            float min[3];
            float max[3];
            float extra[4];
        };

        Header header{};
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        constexpr uint32_t kMaxStaticPropVertices = 1000000;
        bool isRawVertexFile = std::memcmp(header.magic, "BRMPRP1", 7) == 0;
        bool isPackedVertexFileV2 = std::memcmp(header.magic, "BRMPRP2", 7) == 0;
        bool isPackedVertexFile = std::memcmp(header.magic, "BRMPRP3", 7) == 0;
        if (!in || (!isRawVertexFile && !isPackedVertexFileV2 && !isPackedVertexFile) ||
            header.vertexCount == 0 ||
            header.vertexCount > kMaxStaticPropVertices) {
            return false;
        }

        if (isRawVertexFile) {
            if (header.vertexStride != sizeof(Vertex)) return false;
            out.vertices.resize(header.vertexCount);
            in.read(reinterpret_cast<char*>(out.vertices.data()),
                static_cast<std::streamsize>(out.vertices.size() * sizeof(Vertex)));
            if (!in) {
                out = {};
                return false;
            }
        } else if (isPackedVertexFileV2) {
            if (header.vertexStride != sizeof(PackedStaticPropVertexV2)) return false;
            std::vector<PackedStaticPropVertexV2> packed(header.vertexCount);
            in.read(reinterpret_cast<char*>(packed.data()),
                static_cast<std::streamsize>(packed.size() * sizeof(PackedStaticPropVertexV2)));
            if (!in) {
                out = {};
                return false;
            }
            XMFLOAT3 minP{header.min[0], header.min[1], header.min[2]};
            XMFLOAT3 extent{
                header.max[0] - header.min[0],
                header.max[1] - header.min[1],
                header.max[2] - header.min[2]
            };
            auto unpackUnit = [](uint16_t v) {
                return static_cast<float>(v) * (1.0f / 65535.0f);
            };
            auto unpackSnorm = [](int16_t v) {
                return std::clamp(static_cast<float>(v) * (1.0f / 32767.0f), -1.0f, 1.0f);
            };
            out.vertices.reserve(header.vertexCount);
            for (const PackedStaticPropVertexV2& src : packed) {
                XMFLOAT3 pos{
                    minP.x + extent.x * unpackUnit(src.px),
                    minP.y + extent.y * unpackUnit(src.py),
                    minP.z + extent.z * unpackUnit(src.pz)
                };
                XMFLOAT3 normal = Normalize3({unpackSnorm(src.nx), unpackSnorm(src.ny), unpackSnorm(src.nz)}, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 tangent = Normalize3({unpackSnorm(src.tx), unpackSnorm(src.ty), unpackSnorm(src.tz)}, {1.0f, 0.0f, 0.0f});
                out.vertices.push_back({pos, normal, tangent, {src.u, src.v}, static_cast<float>(src.material)});
            }
        } else {
            if (header.vertexStride != sizeof(PackedStaticPropVertex)) return false;
            std::vector<PackedStaticPropVertex> packed(header.vertexCount);
            in.read(reinterpret_cast<char*>(packed.data()),
                static_cast<std::streamsize>(packed.size() * sizeof(PackedStaticPropVertex)));
            if (!in) {
                out = {};
                return false;
            }
            XMFLOAT3 minP{header.min[0], header.min[1], header.min[2]};
            XMFLOAT3 extent{
                header.max[0] - header.min[0],
                header.max[1] - header.min[1],
                header.max[2] - header.min[2]
            };
            float uvMinU = header.extra[0];
            float uvMinV = header.extra[1];
            float uvExtentU = header.extra[2] - header.extra[0];
            float uvExtentV = header.extra[3] - header.extra[1];
            auto unpackUnit = [](uint16_t v) {
                return static_cast<float>(v) * (1.0f / 65535.0f);
            };
            auto unpackSnorm = [](int16_t v) {
                return std::clamp(static_cast<float>(v) * (1.0f / 32767.0f), -1.0f, 1.0f);
            };
            out.vertices.reserve(header.vertexCount);
            for (const PackedStaticPropVertex& src : packed) {
                XMFLOAT3 pos{
                    minP.x + extent.x * unpackUnit(src.px),
                    minP.y + extent.y * unpackUnit(src.py),
                    minP.z + extent.z * unpackUnit(src.pz)
                };
                XMFLOAT3 normal = Normalize3({unpackSnorm(src.nx), unpackSnorm(src.ny), unpackSnorm(src.nz)}, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 tangent = Normalize3({unpackSnorm(src.tx), unpackSnorm(src.ty), unpackSnorm(src.tz)}, {1.0f, 0.0f, 0.0f});
                XMFLOAT2 uv{
                    uvMinU + uvExtentU * unpackUnit(src.u),
                    uvMinV + uvExtentV * unpackUnit(src.v)
                };
                out.vertices.push_back({pos, normal, tangent, uv, static_cast<float>(src.material)});
            }
        }
        out.min = {header.min[0], header.min[1], header.min[2]};
        out.max = {header.max[0], header.max[1], header.max[2]};
        out.generatedUvFallback = StaticPropNeedsGeneratedUv(out);
        return true;
    }

    bool LoadStaticPropObj(const std::wstring& configuredPath, float material, StaticPropMesh& out) const {
        out = {};
        std::filesystem::path path = ResolveConfiguredAssetPath(configuredPath);
        if (path.empty()) {
            std::filesystem::path fallbackConfigured(configuredPath);
            std::wstring ext = fallbackConfigured.extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
            if (ext == L".brmesh") {
                fallbackConfigured.replace_extension(L".obj");
                path = ResolveConfiguredAssetPath(fallbackConfigured.wstring());
            }
        }
        if (path.empty()) {
            StartupProfileLine(L"Prop mesh not found: " + configuredPath);
            return false;
        }

        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        auto applyDefaultMaterialIfUnassigned = [&]() {
            bool hasAssignedMaterial = false;
            for (const Vertex& v : out.vertices) {
                if (std::floor(v.material) > 0.5f) {
                    hasAssignedMaterial = true;
                    break;
                }
            }
            if (!hasAssignedMaterial) {
                for (Vertex& v : out.vertices) {
                    v.material = material;
                }
            }
        };
        if (ext == L".brmesh") {
            if (LoadStaticPropBinary(path, out)) {
                applyDefaultMaterialIfUnassigned();
                return true;
            }

            std::filesystem::path fallbackPath = path;
            fallbackPath.replace_extension(L".obj");
            std::error_code ec;
            if (std::filesystem::exists(fallbackPath, ec)) {
                out = {};
                path = fallbackPath;
            } else {
                StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
                return false;
            }
        } else if (ext == L".bin") {
            if (LoadStaticPropBinary(path, out)) {
                applyDefaultMaterialIfUnassigned();
                return true;
            }
            StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
            return false;
        }

        std::ifstream in(path);
        if (!in) return false;

        std::vector<XMFLOAT3> positions;
        std::vector<XMFLOAT2> texcoords;
        positions.reserve(20000);
        texcoords.reserve(20000);
        XMFLOAT3 minP{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT3 maxP{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        std::string line;
        while (std::getline(in, line)) {
            if (line.size() >= 3 && line[0] == 'v' && line[1] == 't' && std::isspace(static_cast<unsigned char>(line[2]))) {
                const char* p = line.c_str() + 3;
                char* end = nullptr;
                float u = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float v = std::strtof(p, &end);
                if (end == p) continue;
                texcoords.push_back({u, v});
                continue;
            }
            if (line.size() < 2 || line[0] != 'v' || line[1] != ' ') continue;
            const char* p = line.c_str() + 2;
            char* end = nullptr;
            float x = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float y = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float z = std::strtof(p, &end);
            if (end == p) continue;
            positions.push_back({x, y, z});
            minP.x = std::min(minP.x, x);
            minP.y = std::min(minP.y, y);
            minP.z = std::min(minP.z, z);
            maxP.x = std::max(maxP.x, x);
            maxP.y = std::max(maxP.y, y);
            maxP.z = std::max(maxP.z, z);
        }
        if (positions.empty()) return false;

        auto uvFor = [&](XMFLOAT3 p, XMFLOAT3 n, int texcoord) {
            if (texcoord >= 0 && texcoord < static_cast<int>(texcoords.size())) {
                return texcoords[static_cast<size_t>(texcoord)];
            }
            if (std::abs(n.y) > 0.62f) return XMFLOAT2{p.x * 1.7f + 0.5f, p.z * 1.7f + 0.5f};
            if (std::abs(n.x) > std::abs(n.z)) return XMFLOAT2{p.z * 1.7f + 0.5f, p.y * 1.7f};
            return XMFLOAT2{p.x * 1.7f + 0.5f, p.y * 1.7f};
        };
        auto addTri = [&](ObjFaceVertex va, ObjFaceVertex vb, ObjFaceVertex vc, float faceMaterial) {
            XMFLOAT3 a = positions[static_cast<size_t>(va.vertex)];
            XMFLOAT3 b = positions[static_cast<size_t>(vb.vertex)];
            XMFLOAT3 c = positions[static_cast<size_t>(vc.vertex)];
            XMFLOAT3 n = Normalize3(Cross3(Sub3(b, a), Sub3(c, a)), {0.0f, 1.0f, 0.0f});
            XMFLOAT3 tangent = Normalize3(Sub3(b, a), {1.0f, 0.0f, 0.0f});
            if (std::abs(Dot3(tangent, n)) > 0.92f) {
                tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
            }
            out.vertices.push_back({a, n, tangent, uvFor(a, n, va.texcoord), faceMaterial});
            out.vertices.push_back({b, n, tangent, uvFor(b, n, vb.texcoord), faceMaterial});
            out.vertices.push_back({c, n, tangent, uvFor(c, n, vc.texcoord), faceMaterial});
        };

        std::ifstream faces(path);
        if (!faces) return false;
        float currentMaterial = material;
        while (std::getline(faces, line)) {
            if (line.rfind("usemtl ", 0) == 0) {
                const char* p = line.c_str() + 7;
                while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
                currentMaterial = material;
                if (*p == 'p' || *p == 'P') {
                    char* end = nullptr;
                    long id = std::strtol(p + 1, &end, 10);
                    if (end != p + 1 && id >= 0 && id < kMaterialCount) {
                        currentMaterial = static_cast<float>(id);
                    }
                }
                continue;
            }
            if (line.size() < 2 || line[0] != 'f' || line[1] != ' ') continue;
            std::vector<ObjFaceVertex> poly;
            poly.reserve(8);
            const char* p = line.c_str() + 2;
            while (*p) {
                ObjFaceVertex fv = ParseObjFaceVertex(p, static_cast<int>(positions.size()), static_cast<int>(texcoords.size()));
                if (fv.vertex >= 0 && fv.vertex < static_cast<int>(positions.size())) poly.push_back(fv);
            }
            if (poly.size() < 3) continue;
            for (size_t i = 1; i + 1 < poly.size(); ++i) {
                addTri(poly[0], poly[i], poly[i + 1], currentMaterial);
            }
        }

        if (out.vertices.empty()) return false;
        out.min = minP;
        out.max = maxP;
        out.generatedUvFallback = StaticPropNeedsGeneratedUv(out);
        return true;
    }

    bool LoadPropMeshes() {
        propMeshesLoaded_ = false;
        for (StaticPropMesh& mesh : chairPropMeshes_) mesh = {};
        cabinetPropMesh_ = {};
        deskPropMesh_ = {};
        trashBinPropMesh_ = {};
        deskLampPropMesh_ = {};
        cassettePropMesh_ = {};
        airVentPropMesh_ = {};
        exitSignPropMesh_ = {};
        for (StaticPropMesh& mesh : ceilingLampPropMeshes_) mesh = {};

        int loaded = 0;
        const std::array<std::wstring, 3> chairPaths = {
            L"assets\\models\\runtime\\office_chair_modern.brmesh",
            L"assets\\models\\runtime\\office_chair_classic.brmesh",
            L"assets\\models\\runtime\\office_chair_task.brmesh"
        };
        const std::array<float, 3> chairMaterials = {16.0f, 20.0f, 22.0f};
        for (size_t i = 0; i < chairPaths.size(); ++i) {
            if (LoadStaticPropObj(chairPaths[i], chairMaterials[i], chairPropMeshes_[i])) {
                for (Vertex& v : chairPropMeshes_[i].vertices) {
                    int materialId = static_cast<int>(std::floor(v.material));
                    if (materialId == 19 || materialId == 20 || materialId == 21) {
                        v.material = chairMaterials[i];
                    }
                }
                ++loaded;
            }
        }
        bool cabinetLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\filing_cabinet.brmesh", 10.0f, cabinetPropMesh_);
        bool deskLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\office_desk.brmesh", 8.0f, deskPropMesh_);
        bool trashBinLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\trashbin.brmesh", 10.0f, trashBinPropMesh_);
        bool deskLampLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\desklamp.brmesh", 21.0f, deskLampPropMesh_);
        bool cassetteLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\audio_caset.brmesh", 23.0f, cassettePropMesh_);
        bool airVentLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\air_vent.brmesh", 10.0f, airVentPropMesh_);
        bool exitSignLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\emergency_exit_sign.brmesh", 7.0f, exitSignPropMesh_);
        int ceilingLampLoaded = 0;
        for (size_t i = 0; i < ceilingLampPropMeshes_.size(); ++i) {
            wchar_t path[96]{};
            swprintf_s(path, L"assets\\models\\runtime\\ceiling_lamp_%02zu.brmesh", i + 1);
            if (LoadStaticPropObj(path, 21.0f, ceilingLampPropMeshes_[i])) {
                ++ceilingLampLoaded;
            }
        }
        if (cabinetLoaded) ++loaded;
        if (deskLoaded) ++loaded;
        if (trashBinLoaded) ++loaded;
        if (deskLampLoaded) ++loaded;
        if (cassetteLoaded) ++loaded;
        if (airVentLoaded) ++loaded;
        if (exitSignLoaded) ++loaded;
        loaded += ceilingLampLoaded;
        StartupProfileLine(L"Loaded prop meshes: " + std::to_wstring(loaded) + L"/14");
        {
            auto tris = [](const StaticPropMesh& mesh) {
                return static_cast<uint64_t>(mesh.vertices.size() / 3);
            };
            uint64_t chairTris = 0;
            for (const StaticPropMesh& mesh : chairPropMeshes_) chairTris += tris(mesh);
            uint64_t ceilingLampTris = 0;
            for (const StaticPropMesh& mesh : ceilingLampPropMeshes_) ceilingLampTris += tris(mesh);
            std::wstringstream counts;
            counts << L"Prop mesh triangle library: chairs=" << chairTris
                << L", ceilingLamps=" << ceilingLampTris
                << L", cabinet=" << tris(cabinetPropMesh_)
                << L", desk=" << tris(deskPropMesh_)
                << L", trashBin=" << tris(trashBinPropMesh_)
                << L", deskLamp=" << tris(deskLampPropMesh_)
                << L", cassette=" << tris(cassettePropMesh_)
                << L", airVent=" << tris(airVentPropMesh_)
                << L", exitSign=" << tris(exitSignPropMesh_);
            StartupProfileLine(counts.str());
        }
        propMeshesLoaded_ = loaded > 0;
        menuPropMeshesLoaded_ = menuPropMeshesLoaded_ || exitSignLoaded;
        return propMeshesLoaded_;
    }

    bool LoadMenuPropMeshes() {
        if (menuPropMeshesLoaded_ && !exitSignPropMesh_.vertices.empty()) return true;
        exitSignPropMesh_ = {};
        bool exitSignLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\emergency_exit_sign.brmesh", 7.0f, exitSignPropMesh_);
        menuPropMeshesLoaded_ = exitSignLoaded;
        StartupProfileLine(exitSignLoaded ? L"Loaded menu prop mesh: emergency exit sign" : L"Menu exit sign mesh missing");
        return exitSignLoaded;
    }
