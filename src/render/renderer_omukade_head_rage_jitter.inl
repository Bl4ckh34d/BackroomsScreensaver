        if (canTrackPlayer && headLock > 0.55f && visualPlayerLock) {
            XMFLOAT3 cameraFocus{playerPosition.x, playerPosition.y + 0.04f, playerPosition.z};
            XMFLOAT3 toPlayer = Normalize3(Sub3(cameraFocus, skull), hForward);
            float centered = SmoothStep(std::cos(7.0f * kPi / 180.0f), std::cos(1.2f * kPi / 180.0f), Dot3(hForward, toPlayer));
            float rage = centered * SmoothStep(0.55f, 1.0f, headLock);
            if (rage > 0.001f) {
                float yawJitter = (std::sin(timeRuntime_.time * 24.0f + monsterPosition.x * 3.1f) * 0.005f +
                    std::sin(timeRuntime_.time * 39.0f + monsterPosition.z * 1.7f) * 0.003f) * rage;
                float pitchJitter = (std::sin(timeRuntime_.time * 31.0f + monsterPosition.z * 2.4f) * 0.004f +
                    std::sin(timeRuntime_.time * 47.0f + monsterPosition.x * 1.6f) * 0.002f) * rage;

                hForward = Normalize3(Add3(Scale3(hForward, std::cos(yawJitter)), Scale3(hRight, std::sin(yawJitter))), hForward);
                hRight = Normalize3(Cross3(hUp, hForward), hRight);
                hForward = Normalize3(Add3(Scale3(hForward, std::cos(pitchJitter)), Scale3(hUp, std::sin(pitchJitter))), hForward);
                hUp = Normalize3(Cross3(hForward, hRight), hUp);
                hRight = Normalize3(Cross3(hUp, hForward), hRight);
                hUp = Normalize3(Cross3(hForward, hRight), hUp);
                skull = Add3(skull, Add3(Scale3(hRight, std::sin(timeRuntime_.time * 41.0f) * 0.006f * rage),
                    Scale3(hUp, std::sin(timeRuntime_.time * 53.0f + 1.7f) * 0.005f * rage)));
            }
        }
        keepHeadOnSurface(!visualPlayerLock);
