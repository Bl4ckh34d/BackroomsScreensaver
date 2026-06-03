// Renderer mesh path cache helpers.

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
            std::filesystem::path resolved = ResolveAsset(settingsRuntime_.live, configuredPath);
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
        const char* version = "BackroomsMazeMonsterMeshCacheV13";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, &settingsRuntime_.live.monsterSkullMaxTriangles, sizeof(settingsRuntime_.live.monsterSkullMaxTriangles));
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
