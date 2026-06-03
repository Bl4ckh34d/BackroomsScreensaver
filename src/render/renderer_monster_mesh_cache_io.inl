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
