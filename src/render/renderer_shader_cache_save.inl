    static void SaveShaderCache(const char* entry, uint64_t hash, ID3DBlob* blob) {
        if (!blob || blob->GetBufferSize() > 16u * 1024u * 1024u) return;
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t size;
            uint32_t reserved;
        };
        Header header{{'B', 'R', 'M', 'C', 'S', 'O', '1', '\0'}, hash, static_cast<uint32_t>(blob->GetBufferSize()), 0};
        std::ofstream out(ShaderCachePath(entry, hash), std::ios::binary | std::ios::trunc);
        if (!out) return;
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(static_cast<const char*>(blob->GetBufferPointer()), blob->GetBufferSize());
    }
