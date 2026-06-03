    bool LoadOmukadeMaskMesh() {
        renderAssets_.skullMesh.clear();
        renderAssets_.monsterMeshLoaded = false;
        renderAssets_.monsterUsingAltSkull = false;
        renderAssets_.monsterSkullNativeMaskAxes = false;
        std::wstring configuredPath = settingsRuntime_.live.monsterSkullMesh;
        if (!settingsRuntime_.live.monsterAltSkullMesh.empty() &&
            Rand01(913, 917, sessionRuntime_.runtimeSeed) < std::clamp(settingsRuntime_.live.monsterAltSkullChance, 0.0f, 1.0f)) {
            configuredPath = settingsRuntime_.live.monsterAltSkullMesh;
            renderAssets_.monsterUsingAltSkull = true;
        }
        const std::wstring defaultMaskPath = L"assets\\models\\monster_face_mask\\horror_mask.obj";
        std::filesystem::path path = ResolveConfiguredAssetPath(configuredPath);
        if (path.empty() && configuredPath != defaultMaskPath) {
            configuredPath = defaultMaskPath;
            renderAssets_.monsterUsingAltSkull = false;
            path = ResolveConfiguredAssetPath(configuredPath);
        }
        if (path.empty()) {
            StartupProfileLine(L"Monster skull mesh not found: " + configuredPath);
            return false;
        }
        renderAssets_.monsterSkullNativeMaskAxes = IsNativeMaskMeshPath(path);
        uint64_t hash = MonsterMeshCacheHash(path);
        if (LoadMonsterMeshCache(hash, renderAssets_.skullMesh)) {
            StartupProfileLine(L"Loaded cached monster skull mesh: " + std::to_wstring(renderAssets_.skullMesh.size() / 3) + L" tris");
            renderAssets_.monsterMeshLoaded = true;
            return true;
        }

        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        bool ok = false;
        if (ext == L".bin") {
            ok = LoadMonsterMeshFile(path, renderAssets_.skullMesh);
        } else if (ext == L".obj") {
            ok = LoadOmukadeMaskObj(path, renderAssets_.skullMesh);
        }
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

        if (ok) {
            renderAssets_.monsterMeshLoaded = true;
            if (ext != L".bin") {
                SaveMonsterMeshCache(hash, renderAssets_.skullMesh);
            }
            StartupProfileLine(L"Loaded monster skull mesh: " + path.wstring() + L" (" + std::to_wstring(renderAssets_.skullMesh.size() / 3) + L" tris)");
        } else {
            renderAssets_.skullMesh.clear();
            StartupProfileLine(L"Could not load monster skull mesh: " + path.wstring());
        }
        return ok;
    }
