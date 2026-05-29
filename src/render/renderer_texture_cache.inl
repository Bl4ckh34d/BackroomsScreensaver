    static uint64_t HashWide(uint64_t hash, const std::wstring& text) {
        return Fnv1aAppend(hash, text.data(), text.size() * sizeof(wchar_t));
    }

    static uint64_t HashFileSignature(uint64_t hash, const std::filesystem::path& path) {
        std::error_code ec;
        uintmax_t size = std::filesystem::file_size(path, ec);
        hash = Fnv1aAppend(hash, &size, sizeof(size));
        std::ifstream in(path, std::ios::binary);
        if (!in) return hash;

        constexpr std::streamoff kSignatureChunk = 64 * 1024;
        std::array<char, static_cast<size_t>(kSignatureChunk)> buffer{};
        auto readChunk = [&](std::streamoff offset, std::streamsize wanted) {
            if (wanted <= 0) return;
            in.clear();
            in.seekg(offset, std::ios::beg);
            in.read(buffer.data(), wanted);
            std::streamsize read = in.gcount();
            if (read > 0) {
                hash = Fnv1aAppend(hash, buffer.data(), static_cast<size_t>(read));
            }
        };

        std::streamsize firstBytes = static_cast<std::streamsize>(std::min<uintmax_t>(size, static_cast<uintmax_t>(kSignatureChunk)));
        readChunk(0, firstBytes);
        if (size > static_cast<uintmax_t>(kSignatureChunk)) {
            std::streamoff lastOffset = static_cast<std::streamoff>(size - static_cast<uintmax_t>(kSignatureChunk));
            readChunk(lastOffset, static_cast<std::streamsize>(kSignatureChunk));
        }
        return hash;
    }

    uint64_t TextureCacheHash() const {
        uint64_t hash = 1469598103934665603ull;
        const char* version = "BackroomsMazeTextureCacheV16";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, &kTextureSize, sizeof(kTextureSize));
        hash = Fnv1aAppend(hash, &kMaterialCount, sizeof(kMaterialCount));
        hash = HashWide(hash, settings_.wallStem);
        hash = HashWide(hash, settings_.floorStem);
        hash = HashWide(hash, settings_.ceilingStem);
        hash = HashWide(hash, settings_.fleshStem);
        hash = Fnv1aAppend(hash, &settings_.useExternalNormals, sizeof(settings_.useExternalNormals));
        hash = Fnv1aAppend(hash, &settings_.maxNormalMapMB, sizeof(settings_.maxNormalMapMB));

        auto addResolvedAsset = [&](const std::wstring& logicalName, const std::filesystem::path& path) {
            hash = HashWide(hash, logicalName);
            std::error_code ec;
            bool exists = !path.empty() && std::filesystem::exists(path, ec);
            hash = Fnv1aAppend(hash, &exists, sizeof(exists));
            if (exists) {
                hash = HashFileSignature(hash, path);
            }
        };

        auto addPbrAsset = [&](const std::wstring& stem, const wchar_t* suffix) {
            if (stem.empty()) {
                uint8_t missing = 0;
                hash = Fnv1aAppend(hash, &missing, sizeof(missing));
                return;
            }
            std::wstring logicalName = stem + suffix;
            addResolvedAsset(logicalName, ResolveAsset(settings_, logicalName));
        };

        const std::wstring stems[] = {settings_.wallStem, settings_.floorStem, settings_.ceilingStem, settings_.fleshStem};
        for (const std::wstring& stem : stems) {
            addPbrAsset(stem, L"_color_4k.jpg");
            addPbrAsset(stem, L"_height_4k.png");
            addPbrAsset(stem, L"_normal_directx_4k.png");
            addPbrAsset(stem, L"_ao_4k.jpg");
            addPbrAsset(stem, L"_roughness_4k.jpg");
        }

        const wchar_t* runtimeTextures[] = {
            L"assets\\models\\runtime\\textures\\emergency_exit_sign_diffuse.jpeg",
            L"assets\\models\\runtime\\textures\\office_chair_modern_diffuse.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_2209.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_textiles.png",
            L"assets\\models\\runtime\\textures\\office_chair_task_diffuse.png",
            L"assets\\models\\monster_face_mask\\horror_mask_baseColor.png",
            L"assets\\models\\monster_face_mask\\horror_mask_normal.png",
            L"assets\\models\\monster_face_mask\\horror_mask_metallicRoughness.png"
        };
        for (const wchar_t* texture : runtimeTextures) {
            addResolvedAsset(texture, ResolveConfiguredAssetPath(texture));
        }
        return hash;
    }

    static std::wstring TextureCacheFileName() {
#if defined(BACKROOMS_GAME_EXE)
        return L"BackroomsMazeGame_textures.bin";
#else
        return L"BackroomsMaze_textures.bin";
#endif
    }

    static std::filesystem::path TextureCachePath() {
        return CacheDirectory() / TextureCacheFileName();
    }

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
