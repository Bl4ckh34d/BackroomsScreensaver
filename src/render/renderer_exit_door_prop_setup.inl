
        if (!exitPortal.valid) return;
        float bx = exitPortal.wallCenter.x;
        float bz = exitPortal.wallCenter.z;
        XMFLOAT3 inward = exitPortal.inward;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 right = exitPortal.right;
        constexpr float fixedDoorHalfW = 0.60f;
        const bool menuExitDoor = sessionRuntime_.mode == RendererRuntimeMode::MainMenu;
        constexpr float fixedDoorHalfH = 1.05f;
        constexpr float fixedFramePostHalfW = 0.055f;
        constexpr float fixedFrameTopHalfH = 0.070f;
        constexpr float fixedFrameDepthHalf = 0.022f;
        constexpr float fixedFrameForwardOffset = -0.012f;
        constexpr float fixedFrameMaterial = 21.37f;
        const float fixedDoorCenterY = menuExitDoor ? 1.068f : 1.05f;
        XMFLOAT3 doorCenter{bx + inward.x * 0.026f, fixedDoorCenterY, bz + inward.z * 0.026f};
        XMFLOAT3 forward = inward;
        exitDoorPresentation_.center = doorCenter;
        exitDoorPresentation_.normal = inward;
        exitDoorPresentation_.right = right;
        exitDoorPresentation_.hinge = Add3(doorCenter, OrientedOffset(right, up, forward, -fixedDoorHalfW, 0.0f, 0.0f));
        constexpr float framePostCenterX = fixedDoorHalfW + fixedFramePostHalfW;
        const float framePostHalfH = (fixedDoorCenterY + fixedDoorHalfH + fixedFrameTopHalfH * 2.0f) * 0.5f;
        const float framePostCenterY = framePostHalfH;
        const float frameTopCenterY = fixedDoorCenterY + fixedDoorHalfH + fixedFrameTopHalfH;
        constexpr float frameOuterHalfW = fixedDoorHalfW + fixedFramePostHalfW * 2.0f;
        auto addDoorFrameBox = [&](XMFLOAT3 boxCenter, XMFLOAT3 half, bool capBottom) {
            auto p = [&](float x, float y, float z) {
                return Add3(boxCenter, OrientedOffset(right, up, forward, x * half.x, y * half.y, z * half.z));
            };
            auto face = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c0, XMFLOAT3 d, XMFLOAT3 n, XMFLOAT3 t) {
                AddQuadUV(build.vertices, build.indices, a, b, c0, d, n, t, {0, 0}, {1, 0}, {1, 1}, {0, 1}, fixedFrameMaterial);
            };
            face(p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right);
            face(p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f));
            face(p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward);
            face(p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right);
            if (capBottom) {
                face(p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right);
            }
        };
