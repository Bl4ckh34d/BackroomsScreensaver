        if (positions.empty()) return false;

        XMFLOAT3 center{(minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f, (minP.z + maxP.z) * 0.5f};
        float maxDim = std::max({maxP.x - minP.x, maxP.y - minP.y, maxP.z - minP.z, 0.001f});
        float scale = 1.12f / maxDim;
        bool nativeMaskAxes = IsNativeMaskMeshPath(path);
        auto localPos = [&](int idx) {
            const XMFLOAT3& p = positions[static_cast<size_t>(idx)];
            if (nativeMaskAxes) {
                return XMFLOAT3{(p.x - center.x) * scale, (p.z - center.z) * scale, -(p.y - center.y) * scale};
            }
            return XMFLOAT3{(p.x - center.x) * scale, (p.z - center.z) * scale, (p.y - center.y) * scale};
        };
        auto localNormal = [&](int idx) {
            const XMFLOAT3& n = normals[static_cast<size_t>(idx)];
            if (nativeMaskAxes) return Normalize3({n.x, n.z, -n.y}, {0.0f, 0.0f, 1.0f});
            return Normalize3({n.x, n.z, n.y}, {0.0f, 0.0f, 1.0f});
        };
