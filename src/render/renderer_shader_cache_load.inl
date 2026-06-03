    static bool LoadShaderCache(const char* entry, uint64_t hash, ComPtr<ID3DBlob>& blob) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t size;
            uint32_t reserved;
        };
        std::filesystem::path writablePath = ShaderCachePath(entry, hash);
        std::filesystem::path bundledPath = ModuleDirectory() / L"ShaderCache" / ShaderCacheFileName(entry, hash);
        std::filesystem::path legacyWritablePath = CacheDirectory() / LegacyShaderCacheFileName(entry);
        std::filesystem::path legacyBundledPath = ModuleDirectory() / L"ShaderCache" / LegacyShaderCacheFileName(entry);
        const std::array<std::filesystem::path, 4> candidates = {writablePath, bundledPath, legacyWritablePath, legacyBundledPath};
        for (const auto& candidate : candidates) {
            Header header{};
            std::ifstream in(candidate, std::ios::binary);
            if (!in) continue;
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (!in || std::memcmp(header.magic, "BRMCSO1", 7) != 0 || header.hash != hash || header.size < 4 || header.size > 16u * 1024u * 1024u) {
                continue;
            }
            ComPtr<ID3DBlob> loaded;
            if (FAILED(D3DCreateBlob(header.size, &loaded))) continue;
            in.read(static_cast<char*>(loaded->GetBufferPointer()), header.size);
            if (!in) continue;
            if (std::memcmp(loaded->GetBufferPointer(), "DXBC", 4) != 0) continue;
            blob = loaded;
            return true;
        }
        return false;
    }
