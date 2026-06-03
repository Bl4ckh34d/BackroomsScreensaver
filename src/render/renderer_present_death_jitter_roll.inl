        if (world.deathActive) {
            float jitterStrength = (1.0f - SmoothStep(0.74f, 1.0f, deathProgress)) * (0.010f + deathProgress * 0.025f);
            float jitterX = (std::sin(timeRuntime_.time * 31.0f) + std::sin(timeRuntime_.time * 57.0f) * 0.45f) * jitterStrength;
            float jitterY = (std::sin(timeRuntime_.time * 43.0f) + std::sin(timeRuntime_.time * 71.0f) * 0.35f) * jitterStrength;
            viewDir = XMVector3Normalize(viewDir + viewRight * jitterX + worldUp * jitterY);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
        float tunnelRoll = world.playerTunnelLeanSide * SmoothStep(0.0f, 1.0f, world.playerTunnelLeanAmount) * 0.115f;
        float roll = world.deathActive
            ? (std::sin(timeRuntime_.time * 18.0f) * 0.075f + std::sin(timeRuntime_.time * 37.0f) * 0.045f) * SmoothStep(0.06f, 0.42f, deathProgress)
            : viewRuntime_.stumbleYawOffset * 0.12f * SmoothStep(0.0f, 0.35f, viewRuntime_.stumbleTimer) + tunnelRoll;
        XMVECTOR up = XMVector3Normalize(worldUp * std::cos(roll) + viewRight * std::sin(roll));
        XMVECTOR at = eye + viewDir;
        XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
