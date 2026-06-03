        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        bool ok = false;
        if (ext == L".bin") {
            ok = LoadMonsterMeshFile(path, renderAssets_.skullMesh);
        } else if (ext == L".obj") {
            ok = LoadOmukadeMaskObj(path, renderAssets_.skullMesh);
        }
