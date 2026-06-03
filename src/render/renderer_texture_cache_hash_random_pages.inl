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
