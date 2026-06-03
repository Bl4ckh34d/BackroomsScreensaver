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
