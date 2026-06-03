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
