    static XMFLOAT3 TransformInstancePoint(XMFLOAT3 origin,
                                           XMFLOAT3 axisX,
                                           XMFLOAT3 axisY,
                                           XMFLOAT3 axisZ,
                                           XMFLOAT3 p) {
        return Add3(origin, Add3(Scale3(axisX, p.x), Add3(Scale3(axisY, p.y), Scale3(axisZ, p.z))));
    }

    bool AddInstancedStaticProp(const StaticPropMesh& mesh,
                                XMFLOAT3 origin,
                                float yaw,
                                float scaleX,
                                float scaleY,
                                float scaleZ,
                                std::vector<Vertex>& instancedVertices,
                                std::vector<uint32_t>& instancedIndices,
                                std::vector<PendingStaticInstance>& pendingStaticInstances,
                                std::vector<InstancedMeshRange>& instancedMeshRanges,
                                float pitch = 0.0f,
                                float materialOverride = -1.0f,
                                bool castsShadow = false,
                                float materialVariant = 0.0f) {
        int meshId = RegisterInstancedStaticMesh(mesh, instancedVertices, instancedIndices, instancedMeshRanges);
        if (meshId < 0) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);
        XMFLOAT3 axisX = Scale3(right, scaleX);
        XMFLOAT3 axisY = Scale3(up, scaleY);
        XMFLOAT3 axisZ = Scale3(forward, scaleZ);

        const InstancedMeshRange& range = instancedMeshRanges[static_cast<size_t>(meshId)];
        XMFLOAT3 minP{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT3 maxP{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        for (int xi = 0; xi <= 1; ++xi) {
            for (int yi = 0; yi <= 1; ++yi) {
                for (int zi = 0; zi <= 1; ++zi) {
                    XMFLOAT3 local{
                        xi ? range.max.x : range.min.x,
                        yi ? range.max.y : range.min.y,
                        zi ? range.max.z : range.min.z
                    };
                    XMFLOAT3 p = TransformInstancePoint(origin, axisX, axisY, axisZ, local);
                    minP.x = std::min(minP.x, p.x);
                    minP.y = std::min(minP.y, p.y);
                    minP.z = std::min(minP.z, p.z);
                    maxP.x = std::max(maxP.x, p.x);
                    maxP.y = std::max(maxP.y, p.y);
                    maxP.z = std::max(maxP.z, p.z);
                }
            }
        }

        PendingStaticInstance pending{};
        pending.meshId = static_cast<UINT>(meshId);
        pending.data.axisXOriginX = {axisX.x, axisX.y, axisX.z, origin.x};
        pending.data.axisYOriginY = {axisY.x, axisY.y, axisY.z, origin.y};
        pending.data.axisZOriginZ = {axisZ.x, axisZ.y, axisZ.z, origin.z};
        pending.data.materialOverrideVariant = {materialOverride, materialVariant, 0.0f, 0.0f};
        pending.min = minP;
        pending.max = maxP;
        pending.center = {(minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f, (minP.z + maxP.z) * 0.5f};
        float dx = maxP.x - pending.center.x;
        float dy = maxP.y - pending.center.y;
        float dz = maxP.z - pending.center.z;
        pending.radius = std::sqrt(dx * dx + dy * dy + dz * dz);
        TileBoundsForWorldBounds(minP, maxP, pending.minTileX, pending.minTileY, pending.maxTileX, pending.maxTileY);
        pending.castsShadow = castsShadow;
        pendingStaticInstances.push_back(pending);
        return true;
    }
