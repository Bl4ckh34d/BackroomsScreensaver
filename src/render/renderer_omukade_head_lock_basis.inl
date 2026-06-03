        bool visualPlayerLock = canTrackPlayer && MonsterLineOfSightToPlayer();
        float liveLock = canTrackPlayer ? monsterPresentation_.headLockAmount : 0.0f;
        float headLock = std::max(deathHeadLock, liveLock);
        float scanWeight = 1.0f - SmoothStep(0.0f, 1.0f, Clamp01(headLock));
        float headYaw = faceYaw + monsterPresentation_.headYawOffset * 0.24f * scanWeight +
            twitch * 0.32f * (1.0f - deathHeadLock * 0.85f) * scanWeight;
        XMFLOAT3 hRight{std::cos(headYaw), 0.0f, -std::sin(headYaw)};
        XMFLOAT3 hUp = bodyUps[0];
        XMFLOAT3 hForward{std::sin(headYaw), 0.0f, std::cos(headYaw)};
        float uprightBlend = blobSurfaceUp.y < 0.20f ? 0.82f : 0.58f;
        hUp = Normalize3(Lerp3(blobSurfaceUp, {0.0f, 1.0f, 0.0f}, uprightBlend), {0.0f, 1.0f, 0.0f});
        hForward = Normalize3(Sub3(hForward, Scale3(hUp, Dot3(hForward, hUp))), monsterForward);
        if (Length3(hForward) < 0.001f) hForward = Normalize3(Cross3(hUp, monsterRight), monsterForward);
        hRight = Normalize3(Cross3(hUp, hForward), hRight);
        hUp = Normalize3(Cross3(hForward, hRight), hUp);
