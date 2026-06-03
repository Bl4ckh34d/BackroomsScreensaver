    void SetMonsterPreviewCamera(float orbitSeconds = 0.0f) {
        if (!monsterPreview_.active) return;
        XMFLOAT3 target = MonsterFocusPoint();
        XMFLOAT3 cameraPosition{};
        if (monsterPreview_.view != MonsterPreviewView::Top && monsterPreview_.manualOrbit) {
            float cp = std::cos(monsterPreview_.orbitPitch);
            cameraPosition = {
                target.x + std::sin(monsterPreview_.orbitYaw) * cp * monsterPreview_.orbitDistance,
                target.y + std::sin(monsterPreview_.orbitPitch) * monsterPreview_.orbitDistance,
                target.z - std::cos(monsterPreview_.orbitYaw) * cp * monsterPreview_.orbitDistance
            };
        } else if (monsterPreview_.view == MonsterPreviewView::Front) {
            cameraPosition = {target.x, target.y - 0.48f, target.z - 3.25f};
        } else if (monsterPreview_.view == MonsterPreviewView::Side) {
            cameraPosition = {target.x + 3.25f, target.y - 0.48f, target.z};
        } else if (monsterPreview_.view == MonsterPreviewView::LeftSide) {
            cameraPosition = {target.x - 3.25f, target.y - 0.48f, target.z};
        } else if (monsterPreview_.view == MonsterPreviewView::Top) {
            cameraPosition = {target.x, target.y + 4.60f, target.z};
        } else {
            float orbit = orbitSeconds * 0.78f;
            cameraPosition = {
                target.x + std::sin(orbit) * 3.15f,
                target.y - 0.52f,
                target.z - std::cos(orbit) * 3.15f
            };
        }
        float dx = target.x - cameraPosition.x;
        float dy = target.y - cameraPosition.y;
        float dz = target.z - cameraPosition.z;
        float yaw = std::atan2(dx, dz);
        float horizontal = std::sqrt(dx * dx + dz * dz);
        float pitch = monsterPreview_.view == MonsterPreviewView::Top
            ? -20.0f
            : std::clamp(std::atan2(dy, std::max(0.001f, horizontal)), -0.36f, 0.48f);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        gameWorld_.SetPlayerCameraPose(cameraPosition, yaw, world.monsterYaw, pitch);
        viewRuntime_.flashlightYaw = yaw;
        viewRuntime_.flashlightPitch = pitch;
    }
