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
