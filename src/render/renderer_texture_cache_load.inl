    static bool LoadTextureCache(uint64_t hash, std::vector<uint8_t>& albedo, std::vector<uint8_t>& normal, std::vector<uint8_t>& props) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t textureSize;
            uint32_t materialCount;
            uint64_t albedoSize;
            uint64_t normalSize;
            uint64_t propsSize;
        };
        std::filesystem::path bundledPath = ModuleDirectory() / L"TextureCache" / TextureCacheFileName();
        std::filesystem::path writablePath = TextureCachePath();
        const std::array<std::filesystem::path, 2> candidates = {writablePath, bundledPath};
        for (const auto& candidate : candidates) {
            Header header{};
            std::ifstream in(candidate, std::ios::binary);
            if (!in) continue;
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (!in ||
                std::memcmp(header.magic, "BRMTEX2", 7) != 0 ||
                header.hash != hash ||
                header.textureSize != kTextureSize ||
                header.materialCount != kMaterialCount ||
                header.albedoSize != albedo.size() ||
                header.normalSize != normal.size() ||
                header.propsSize != props.size()) {
                continue;
            }
            in.read(reinterpret_cast<char*>(albedo.data()), static_cast<std::streamsize>(albedo.size()));
            if (!in) continue;
            in.read(reinterpret_cast<char*>(normal.data()), static_cast<std::streamsize>(normal.size()));
            if (!in) continue;
            in.read(reinterpret_cast<char*>(props.data()), static_cast<std::streamsize>(props.size()));
            if (in) return true;
        }
        return false;
    }
