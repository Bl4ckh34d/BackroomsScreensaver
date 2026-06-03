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
