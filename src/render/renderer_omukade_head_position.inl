        keepHeadOnSurface();
        float headSurfaceLift = bodyRadii[0] * 0.46f + MonsterHeadBobOffset() * 0.14f * modelY;
        float headForwardLift = bodyRadii[0] * 0.72f + kMonsterHeadForwardOffset * 0.28f * modelXZ;
        XMFLOAT3 headRoot = Add3(bodyPoints[0], OrientedOffset(hRight, hUp, hForward,
            0.0f, bodyRadii[0] * 0.24f, bodyRadii[0] * 0.18f));
        XMFLOAT3 skull = Add3(bodyPoints[0], OrientedOffset(hRight, hUp, hForward,
            0.0f, headSurfaceLift, headForwardLift));
        skull = Add3(skull, Scale3(hUp, curiosityPose * 0.24f * modelY));
