        auto resolvePbrPath = [&](const std::wstring& base, std::initializer_list<const wchar_t*> suffixes) {
            for (const wchar_t* suffix : suffixes) {
                std::filesystem::path path = ResolveAsset(settingsRuntime_.live, base + suffix);
                if (!path.empty()) return path;
            }
            return std::filesystem::path{};
        };
