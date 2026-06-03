        XMFLOAT3 right{std::cos(faceYaw), 0.0f, -std::sin(faceYaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(faceYaw), 0.0f, std::cos(faceYaw)};
        float hover = 0.30f + std::sin(timeRuntime_.time * 1.25f + monsterPosition.x * 0.07f + monsterPosition.z * 0.05f) * 0.030f;
        auto off = [&](float x, float y, float z) {
            return Add3(monsterPosition, OrientedOffset(right, up, forward, x * modelXZ, y * modelY + hover, z * modelXZ));
        };
        auto box = [&](float x, float y, float z, float hx, float hy, float hz, float material) {
            AppendDynamicBoxAxes(solidVerts, off(x, y, z), right, up, forward,
                {hx * modelXZ, hy * modelY, hz * modelXZ}, material);
        };
        auto seg = [&](XMFLOAT3 a, XMFLOAT3 b, float w, float d, float material) {
            AppendSegmentBox(solidVerts, a, b, w * modelXZ, d * modelXZ, material);
        };

        float twitch = std::sin(timeRuntime_.time * 9.4f + monsterPosition.x * 0.3f) * 0.018f;
        float breathe = std::sin(timeRuntime_.time * 2.3f) * 0.030f;
        float deathHeadLock = world.deathActive ? SmoothStep(0.0f, 0.22f, world.deathTimer) : 0.0f;
        constexpr float boneMat = 9.65f;
        constexpr float gutMat = 20.68f;
        constexpr float limbMat = 20.74f;
        constexpr float darkMat = 10.0f;
        constexpr float handprintMat = 25.35f;
