    bool CompileShader(const char* src, const char* entry, const char* profile, ComPtr<ID3DBlob>& blob) {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        uint64_t cacheHash = ShaderCacheHash(src, entry, profile, flags);
        ReportShaderActivity(L"Checking cache for", entry, profile);
        if (LoadShaderCache(entry, cacheHash, blob)) {
            SaveShaderCache(entry, cacheHash, blob.Get());
            ReportShaderComplete(L"Loaded cached shader", entry, profile, false);
            return true;
        }

        ReportShaderActivity(L"Compiling", entry, profile);
        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DCompile(src, std::strlen(src), nullptr, nullptr, nullptr, entry, profile, flags, 0, &blob, &errors);
        if (FAILED(hr) && errors) {
            const char* errorText = static_cast<const char*>(errors->GetBufferPointer());
            OutputDebugStringA(errorText);
            std::string bytes(errorText, errorText + errors->GetBufferSize());
            StartupProfileLine(L"Shader compile failed for " + std::wstring(entry, entry + std::strlen(entry)) + L": " + std::wstring(bytes.begin(), bytes.end()));
        }
        if (SUCCEEDED(hr)) {
            SaveShaderCache(entry, cacheHash, blob.Get());
            ReportShaderComplete(L"Compiled shader", entry, profile, true);
        }
        return SUCCEEDED(hr);
    }

    static uint64_t Fnv1aAppend(uint64_t hash, const void* data, size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; ++i) {
            hash ^= bytes[i];
            hash *= 1099511628211ull;
        }
        return hash;
    }

    static uint64_t ShaderCacheHash(const char* src, const char* entry, const char* profile, UINT flags) {
        uint64_t hash = 1469598103934665603ull;
        const char* version = "BackroomsMazeShaderCacheV4";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, entry, std::strlen(entry) + 1);
        hash = Fnv1aAppend(hash, profile, std::strlen(profile) + 1);
        hash = Fnv1aAppend(hash, &flags, sizeof(flags));
        hash = Fnv1aAppend(hash, src, std::strlen(src));
        return hash;
    }

    static std::wstring ShaderCacheSafeEntry(const char* entry) {
        std::wstring safeEntry;
        for (const char* p = entry; *p; ++p) {
            safeEntry.push_back((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')
                ? static_cast<wchar_t>(*p)
                : L'_');
        }
        return safeEntry;
    }

    static std::wstring ShaderCacheFileName(const char* entry, uint64_t hash) {
        std::wstringstream name;
        name << L"BackroomsMaze_" << ShaderCacheSafeEntry(entry) << L"_" << std::hex << hash << L".cso";
        return name.str();
    }

    static std::wstring LegacyShaderCacheFileName(const char* entry) {
        return L"BackroomsMaze_" + ShaderCacheSafeEntry(entry) + L".cso";
    }

    static std::filesystem::path ShaderCachePath(const char* entry, uint64_t hash) {
        return CacheDirectory() / ShaderCacheFileName(entry, hash);
    }

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
