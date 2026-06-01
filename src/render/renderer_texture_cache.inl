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
        const char* version = "BackroomsMazeTextureCacheV21";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, &kTextureSize, sizeof(kTextureSize));
        hash = Fnv1aAppend(hash, &kMaterialCount, sizeof(kMaterialCount));
        hash = HashWide(hash, settings_.assetFolder);
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

        const wchar_t* pbrSuffixes[] = {
            L"_color_4k.jpg", L"_color_4k.png",
            L"_Color.jpg", L"_Color.png",
            L"_BaseColor.jpg", L"_BaseColor.png",
            L"_Albedo.jpg", L"_Albedo.png",
            L"_Diffuse.jpg", L"_Diffuse.png",
            L"_height_4k.png", L"_height_4k.jpg",
            L"_Displacement.jpg", L"_Displacement.png",
            L"_Height.jpg", L"_Height.png",
            L"_normal_directx_4k.png", L"_normal_directx_4k.jpg",
            L"_normal_dx_4k.png", L"_normal_dx_4k.jpg",
            L"_NormalDX.jpg", L"_NormalDX.png",
            L"_NormalDirectX.jpg", L"_NormalDirectX.png",
            L"_normal_opengl_4k.png", L"_normal_opengl_4k.jpg",
            L"_normal_gl_4k.png", L"_normal_gl_4k.jpg",
            L"_NormalGL.jpg", L"_NormalGL.png",
            L"_NormalOpenGL.jpg", L"_NormalOpenGL.png",
            L"_ao_4k.jpg", L"_ao_4k.png",
            L"_AO.jpg", L"_AO.png",
            L"_AmbientOcclusion.jpg", L"_AmbientOcclusion.png",
            L"_roughness_4k.jpg", L"_roughness_4k.png",
            L"_Roughness.jpg", L"_Roughness.png",
            L"_metallic_4k.jpg", L"_metallic_4k.png",
            L"_metalness_4k.jpg", L"_metalness_4k.png",
            L"_Metallic.jpg", L"_Metallic.png",
            L"_Metalness.jpg", L"_Metalness.png"
        };

        const std::wstring stems[] = {settings_.wallStem, settings_.floorStem, settings_.ceilingStem, settings_.fleshStem};
        for (const std::wstring& stem : stems) {
            for (const wchar_t* suffix : pbrSuffixes) {
                addPbrAsset(stem, suffix);
            }
        }

        const wchar_t* runtimeTextures[] = {
            L"assets\\models\\runtime\\textures\\emergency_exit_sign_diffuse.jpeg",
            L"assets\\models\\runtime\\textures\\office_chair_modern_diffuse.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_2209.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_textiles.png",
            L"assets\\models\\runtime\\textures\\office_chair_task_diffuse.png",
            L"assets\\models\\monster_face_mask\\horror_mask_baseColor.png",
            L"assets\\models\\monster_face_mask\\horror_mask_normal.png",
            L"assets\\models\\monster_face_mask\\horror_mask_metallicRoughness.png",
            L"assets\\images\\8pages\\page1.jpg",
            L"assets\\images\\8pages\\page2.jpg",
            L"assets\\images\\8pages\\page3.jpg",
            L"assets\\images\\8pages\\page4.jpg",
            L"assets\\images\\8pages\\page5.jpg",
            L"assets\\images\\8pages\\page6.jpg",
            L"assets\\images\\8pages\\page7.jpg",
            L"assets\\images\\8pages\\page8.jpg"
        };
        for (const wchar_t* texture : runtimeTextures) {
            addResolvedAsset(texture, ResolveConfiguredAssetPath(texture));
        }

        std::filesystem::path randomPages = ResolveConfiguredAssetPath(L"assets\\images\\randomPages");
        std::error_code ec;
        bool randomPageFolderExists = !randomPages.empty() && std::filesystem::exists(randomPages, ec) && std::filesystem::is_directory(randomPages, ec);
        hash = Fnv1aAppend(hash, &randomPageFolderExists, sizeof(randomPageFolderExists));
        if (randomPageFolderExists) {
            std::vector<std::filesystem::path> files;
            for (const auto& entry : std::filesystem::directory_iterator(randomPages, ec)) {
                if (ec || !entry.is_regular_file(ec)) continue;
                std::wstring ext = entry.path().extension().wstring();
                std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
                if (ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".bmp" || ext == L".tif" || ext == L".tiff") {
                    files.push_back(entry.path());
                }
            }
            std::sort(files.begin(), files.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
                return _wcsicmp(a.filename().c_str(), b.filename().c_str()) < 0;
            });
            int fileCount = static_cast<int>(files.size());
            hash = Fnv1aAppend(hash, &fileCount, sizeof(fileCount));
            for (const std::filesystem::path& path : files) {
                hash = HashWide(hash, path.filename().wstring());
                hash = HashFileSignature(hash, path);
            }
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
