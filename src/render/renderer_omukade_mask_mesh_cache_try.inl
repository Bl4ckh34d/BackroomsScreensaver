        uint64_t hash = MonsterMeshCacheHash(path);
        if (LoadMonsterMeshCache(hash, renderAssets_.skullMesh)) {
            StartupProfileLine(L"Loaded cached monster skull mesh: " + std::to_wstring(renderAssets_.skullMesh.size() / 3) + L" tris");
            renderAssets_.monsterMeshLoaded = true;
            return true;
        }
