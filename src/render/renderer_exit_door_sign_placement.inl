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
