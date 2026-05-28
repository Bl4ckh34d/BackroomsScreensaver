    void EnsureFullSceneAssets() {
        if (!monsterMeshLoaded_ || skullMesh_.empty()) {
            LoadMonsterSkullMesh();
        }
        if (!propMeshesLoaded_) {
            LoadPropMeshes();
        }
    }

    static bool StaticPropNeedsGeneratedUv(const StaticPropMesh& mesh) {
        if (mesh.vertices.size() < 3) return false;
        XMFLOAT2 minUv{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT2 maxUv{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        for (const Vertex& v : mesh.vertices) {
            if (!std::isfinite(v.uv.x) || !std::isfinite(v.uv.y)) return true;
            minUv.x = std::min(minUv.x, v.uv.x);
            minUv.y = std::min(minUv.y, v.uv.y);
            maxUv.x = std::max(maxUv.x, v.uv.x);
            maxUv.y = std::max(maxUv.y, v.uv.y);
        }
        return (maxUv.x - minUv.x) < 0.035f || (maxUv.y - minUv.y) < 0.035f;
    }
