        Tile headTile = maze.TileFromWorld(bodyPoints[0].x, bodyPoints[0].z);
        XMFLOAT3 headTileCenter = maze.WorldCenter(headTile, 0.0f);
        float headHalfW = std::max(0.12f, maze.tileW * 0.5f - bodyRadii[0] * 1.35f);
        float headHalfD = std::max(0.12f, maze.tileD * 0.5f - bodyRadii[0] * 1.35f);
        float wallDodgeX = Clamp01(std::abs(bodyPoints[0].x - headTileCenter.x) / headHalfW);
        float wallDodgeZ = Clamp01(std::abs(bodyPoints[0].z - headTileCenter.z) / headHalfD);
        float corridorDodge = std::max(wallDodgeX, wallDodgeZ);
        float dodgeWander = std::sin(timeRuntime_.time * (1.70f + monsterPresentation_.headChaseBlend * 1.60f) + monsterPresentation_.headScanPhase) * 0.022f;
        skull = Add3(skull, Add3(Scale3(hRight, dodgeWander * (1.0f - headLock * 0.55f)),
            Scale3(hUp, corridorDodge * 0.040f * modelY)));
        float headPitch = monsterPresentation_.headPitchOffset * 0.32f * scanWeight;
        if (std::abs(headPitch) > 0.0005f) {
            hForward = Normalize3(Add3(Scale3(hForward, std::cos(headPitch)), Scale3(hUp, std::sin(headPitch))), hForward);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        keepHeadOnSurface();
