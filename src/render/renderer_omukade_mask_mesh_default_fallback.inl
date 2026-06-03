        if (!ok && configuredPath != defaultMaskPath) {
            configuredPath = defaultMaskPath;
            renderAssets_.monsterUsingAltSkull = false;
            path = ResolveConfiguredAssetPath(configuredPath);
            if (!path.empty()) {
                renderAssets_.monsterSkullNativeMaskAxes = IsNativeMaskMeshPath(path);
                hash = MonsterMeshCacheHash(path);
                if (LoadMonsterMeshCache(hash, renderAssets_.skullMesh)) {
                    StartupProfileLine(L"Loaded cached default monster mask mesh: " + std::to_wstring(renderAssets_.skullMesh.size() / 3) + L" tris");
                    renderAssets_.monsterMeshLoaded = true;
                    return true;
                }
                ext = path.extension().wstring();
                std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
                if (ext == L".bin") {
                    ok = LoadMonsterMeshFile(path, renderAssets_.skullMesh);
                } else if (ext == L".obj") {
                    ok = LoadOmukadeMaskObj(path, renderAssets_.skullMesh);
                }
            }
        }
