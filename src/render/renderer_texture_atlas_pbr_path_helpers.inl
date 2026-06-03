        auto resolvePbrPath = [&](const std::wstring& base, std::initializer_list<const wchar_t*> suffixes) {
            for (const wchar_t* suffix : suffixes) {
                std::filesystem::path path = ResolveAsset(settingsRuntime_.live, base + suffix);
                if (!path.empty()) return path;
            }
            return std::filesystem::path{};
        };

        auto loadPbrImage = [&](const std::wstring& base, std::initializer_list<const wchar_t*> suffixes, ImageRGBA& img) {
            std::filesystem::path path = resolvePbrPath(base, suffixes);
            return !path.empty() && LoadImageWic(path, kTextureSize, kTextureSize, img);
        };
