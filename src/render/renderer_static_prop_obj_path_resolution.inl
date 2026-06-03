        out = {};
        std::filesystem::path path = ResolveConfiguredAssetPath(configuredPath);
        if (path.empty()) {
            std::filesystem::path fallbackConfigured(configuredPath);
            std::wstring ext = fallbackConfigured.extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
            if (ext == L".brmesh") {
                fallbackConfigured.replace_extension(L".obj");
                path = ResolveConfiguredAssetPath(fallbackConfigured.wstring());
            }
        }
        if (path.empty()) {
            StartupProfileLine(L"Prop mesh not found: " + configuredPath);
            return false;
        }

        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        auto applyDefaultMaterialIfUnassigned = [&]() {
            bool hasAssignedMaterial = false;
            for (const Vertex& v : out.vertices) {
                if (std::floor(v.material) > 0.5f) {
                    hasAssignedMaterial = true;
                    break;
                }
            }
            if (!hasAssignedMaterial) {
                for (Vertex& v : out.vertices) {
                    v.material = material;
                }
            }
        };
        if (ext == L".brmesh") {
            if (LoadStaticPropBinary(path, out)) {
                applyDefaultMaterialIfUnassigned();
                return true;
            }

            std::filesystem::path fallbackPath = path;
            fallbackPath.replace_extension(L".obj");
            std::error_code ec;
            if (std::filesystem::exists(fallbackPath, ec)) {
                out = {};
                path = fallbackPath;
            } else {
                StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
                return false;
            }
        } else if (ext == L".bin") {
            if (LoadStaticPropBinary(path, out)) {
                applyDefaultMaterialIfUnassigned();
                return true;
            }
            StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
            return false;
        }
