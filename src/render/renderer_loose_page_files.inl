// Renderer texture loose pages helpers.

    std::vector<std::filesystem::path> RandomLoosePageFiles() const {
        std::vector<std::filesystem::path> files;
        std::filesystem::path folder = ResolveConfiguredAssetPath(L"assets\\images\\randomPages");
        std::error_code ec;
        if (!std::filesystem::exists(folder, ec) || !std::filesystem::is_directory(folder, ec)) return files;
        for (const auto& entry : std::filesystem::directory_iterator(folder, ec)) {
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
        return files;
    }
