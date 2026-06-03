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
