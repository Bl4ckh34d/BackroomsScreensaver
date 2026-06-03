    static void SaveTextureCache(uint64_t hash, const std::vector<uint8_t>& albedo, const std::vector<uint8_t>& normal, const std::vector<uint8_t>& props) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t textureSize;
            uint32_t materialCount;
            uint64_t albedoSize;
            uint64_t normalSize;
            uint64_t propsSize;
        };
        Header header{{'B', 'R', 'M', 'T', 'E', 'X', '2', '\0'}, hash, kTextureSize, kMaterialCount, albedo.size(), normal.size(), props.size()};
        std::ofstream out(TextureCachePath(), std::ios::binary | std::ios::trunc);
        if (!out) return;
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(albedo.data()), static_cast<std::streamsize>(albedo.size()));
        out.write(reinterpret_cast<const char*>(normal.data()), static_cast<std::streamsize>(normal.size()));
        out.write(reinterpret_cast<const char*>(props.data()), static_cast<std::streamsize>(props.size()));
    }
