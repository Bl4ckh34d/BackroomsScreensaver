// Static prop placement exit vent cabinet.

    void AddExitDoorPropGeometry(StaticPropPlacementBuildContext& build, const ExitPortal& exitPortal) {
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
        addDoorFrameBox(Add3(doorCenter, OrientedOffset(right, up, forward, -framePostCenterX, framePostCenterY - doorCenter.y, fixedFrameForwardOffset)), {fixedFramePostHalfW, framePostHalfH, fixedFrameDepthHalf}, false);
        addDoorFrameBox(Add3(doorCenter, OrientedOffset(right, up, forward, framePostCenterX, framePostCenterY - doorCenter.y, fixedFrameForwardOffset)), {fixedFramePostHalfW, framePostHalfH, fixedFrameDepthHalf}, false);
        addDoorFrameBox(Add3(doorCenter, OrientedOffset(right, up, forward, 0.0f, frameTopCenterY - doorCenter.y, fixedFrameForwardOffset)), {frameOuterHalfW, fixedFrameTopHalfH, fixedFrameDepthHalf}, true);
        const float fixedSignTargetH = menuExitDoor ? 0.28f : 0.22f;
        const float signBackingHalfH = fixedSignTargetH * 0.5f + 0.016f;
        constexpr float signCeilingClearance = 0.12f;
        const float signForwardOffset = menuExitDoor ? 0.028f : 0.074f;
        float signY = doorCenter.y + fixedDoorHalfH + fixedSignTargetH * 0.5f + 0.24f;
        if (!menuExitDoor) {
            const float frameTopY = frameTopCenterY + fixedFrameTopHalfH;
            signY = frameTopY + signBackingHalfH + 0.120f;
        }
        signY = std::min(signY, build.surface.wallH - signBackingHalfH - signCeilingClearance);
        XMFLOAT3 sign = {bx + forward.x * signForwardOffset, signY, bz + forward.z * signForwardOffset};
        exitDoorPresentation_.signLightPosition = Add3(sign, OrientedOffset(right, up, forward, 0.0f, -0.01f, 0.30f));
        exitDoorPresentation_.signLightStrength = 4.35f;
        float targetW = std::min(1.04f, exitPortal.halfSpan * 1.30f);
        if (!renderAssets_.exitSignPropMesh.vertices.empty()) {
            float spanX = std::max(0.001f, PropMeshSpan(renderAssets_.exitSignPropMesh, 0));
            float spanY = std::max(0.001f, PropMeshSpan(renderAssets_.exitSignPropMesh, 1));
            float targetH = fixedSignTargetH;
            float scale = std::clamp(std::min(targetW / spanX, targetH / spanY), 0.05f, 8.0f);
            float actualSignW = spanX * scale;
            float actualSignH = spanY * scale;
            AddOrientedBox(build.vertices, build.indices, Add3(sign, Scale3(forward, -0.004f)),
                {actualSignW * 0.5f + 0.024f, actualSignH * 0.5f + 0.016f, 0.012f}, exitPortal.yaw, 10.0f);
            XMFLOAT3 localCenter{
                (renderAssets_.exitSignPropMesh.min.x + renderAssets_.exitSignPropMesh.max.x) * 0.5f,
                (renderAssets_.exitSignPropMesh.min.y + renderAssets_.exitSignPropMesh.max.y) * 0.5f,
                (renderAssets_.exitSignPropMesh.min.z + renderAssets_.exitSignPropMesh.max.z) * 0.5f
            };
            auto appendExitSignModel = [&](XMFLOAT3 signCenter, XMFLOAT3 signRight, XMFLOAT3 signForward, float yaw) {
                XMFLOAT3 origin = Add3(signCenter, Add3(Scale3(signRight, -localCenter.x * scale),
                    Add3(Scale3(up, -localCenter.y * scale), Scale3(signForward, -localCenter.z * scale))));
                return AddInstancedStaticProp(renderAssets_.exitSignPropMesh, origin, yaw, scale, scale, scale,
                    build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
                    0.0f, 7.0f);
            };
            bool appended = appendExitSignModel(Add3(sign, Scale3(forward, 0.012f)), right, forward, exitPortal.yaw);
            if (!appended) {
                StartupProfileLine(L"Emergency exit sign mesh was loaded but could not be appended; no handmade fallback was drawn.");
            }
        } else {
            AddOrientedBox(build.vertices, build.indices, Add3(sign, Scale3(forward, 0.012f)),
                {targetW * 0.5f, fixedSignTargetH * 0.5f, 0.014f}, exitPortal.yaw, 7.0f);
            StartupProfileLine(L"Emergency exit sign mesh missing; using handmade fallback.");
        }
    }
