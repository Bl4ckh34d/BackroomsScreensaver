        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        float deathProgress = world.deathActive ? Clamp01(world.deathTimer / 4.25f) : 0.0f;
        float deathFadeProgress = world.deathActive ? Clamp01((world.deathTimer - 0.10f) / 3.55f) : 0.0f;
        float deathFocus = world.deathActive ? SmoothStep(0.0f, 0.24f, world.deathTimer) : 0.0f;
        XMFLOAT3 f = Forward();
        float stepPhase = world.playerStepPhase;
        float runCameraMotion = 1.0f + world.playerRunIntensity * 0.42f + world.playerRunEffort * 0.55f;
        float stepWave = std::sin(stepPhase * 2.0f);
        float sideSway = stepWave * settingsRuntime_.live.sideSwayAmount * 0.26f * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR right = XMVectorSet(std::cos(world.playerBodyYaw), 0.0f, -std::sin(world.playerBodyYaw), 0.0f);
        XMVECTOR eye = XMLoadFloat3(&world.playerPosition) + right * sideSway;
        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        float bobPitch = stepWave * 0.0045f * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR viewDir = XMVector3Normalize(XMVectorSet(f.x, world.playerPitch + bobPitch, f.z, 0.0f));
        XMVECTOR viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        if (monsterPreview_.active && monsterPreview_.view == MonsterPreviewView::Top) {
            XMVECTOR topTarget = XMVectorSet(world.monsterPosition.x, 1.20f, world.monsterPosition.z, 0.0f);
            viewDir = XMVector3Normalize(topTarget - eye);
            worldUp = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
