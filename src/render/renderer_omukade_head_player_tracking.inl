        if (headLock > 0.001f || visualPlayerLock) {
            XMFLOAT3 cameraFocus{playerPosition.x, playerPosition.y + 0.04f, playerPosition.z};
            XMFLOAT3 lookForward = Normalize3(Sub3(cameraFocus, skull), hForward);
            float trackBlend = SmoothStep(0.0f, 1.0f, SmoothStep(0.0f, 1.0f, Clamp01(headLock)));
            float focusBlend = visualPlayerLock
                ? Lerp(0.30f, 0.92f, SmoothStep(0.0f, 1.0f, std::max(headLock, monsterPresentation_.headChaseBlend)))
                : trackBlend * 0.12f;
            hForward = slerpDirection(hForward, lookForward, focusBlend);
            hRight = Normalize3(Cross3(hUp, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        if (curiosityPose > 0.001f && visualPlayerLock) {
            float tilt = std::sin(timeRuntime_.time * 3.45f) * 0.035f * curiosityPose;
            hRight = Normalize3(Add3(Scale3(hRight, std::cos(tilt)), Scale3(hUp, std::sin(tilt))), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        keepHeadOnSurface(!visualPlayerLock);
