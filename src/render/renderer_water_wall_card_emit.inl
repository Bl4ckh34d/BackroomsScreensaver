        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up, -h * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up, -h * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up,  h * 0.5f)));
        XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up,  h * 0.5f)));
        float wallUvBase = sourceFromCeiling ? 3.0f : 4.0f;
        AddQuadUV(build.vertices, build.waterIndices, a, b, c0, d0, normal, right,
            {0, wallUvBase + 0.999f}, {1, wallUvBase + 0.999f},
            {1, wallUvBase + 0.001f}, {0, wallUvBase + 0.001f},
            WaterDecalMaterial(seed, 0.037f, 0.011f));
