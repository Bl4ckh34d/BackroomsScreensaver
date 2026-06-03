    void AddWaterAttentionPoint(const XMFLOAT3& pos,
                                const XMFLOAT3& source,
                                const XMFLOAT3& normal,
                                float radius,
                                float seed,
                                bool requireFacing) {
        if (gEffectDebugViewer || scareRuntime_.bloodScarePoints.size() > 384) return;
        BloodScarePoint scare{};
        scare.pos = pos;
        scare.source = source;
        scare.normal = normal;
        scare.radius = std::clamp(radius, RenderMazeView().TileAverage() * 0.78f, RenderMazeView().TileAverage() * 1.45f);
        scare.focusDelaySeconds = 0.20f + LampHash(seed * 19.0f + pos.x, pos.z - seed * 7.0f) * 0.46f;
        scare.dreadScale = 0.42f;
        scare.requireFacing = requireFacing;
        scare.revealBlood = false;
        scareRuntime_.bloodScarePoints.push_back(scare);
    }
