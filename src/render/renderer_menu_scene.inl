    void ApplyMainMenuSettings() {
        settings_.mazeWidth = 3;
        settings_.mazeHeight = 3;
        settings_.wallHeightMeters = std::max(settings_.wallHeightMeters, 2.85f);
        settings_.mapOverlay = false;
        settings_.debugAiMapOverlay = false;
        settings_.chairDensity = 0.35f;
        settings_.paperDensity = 0.55f;
        settings_.hallwayPaperRunDensity = 0.40f;
        settings_.metalCabinetDensity = 0.0f;
        settings_.waterDamageDensity = 0.0f;
        settings_.lampOnRatio = 1.0f;
        settings_.lampSpacing = std::max(settings_.tileWidthMeters, settings_.tileLengthMeters) * 2.4f;
        settings_.lampIntensity = std::max(settings_.lampIntensity, 1.05f);
        settings_.lampFlickerRatio = 0.0f;
        settings_.brokenZoneRatio = 0.0f;
        settings_.ambientLight = 0.0f;
        settings_.flashlightIntensity = std::max(settings_.flashlightIntensity, 2.78f);
        settings_.flashlightAttenuation = std::min(settings_.flashlightAttenuation, 0.058f);
        settings_.flashlightConeDegrees = std::clamp(settings_.flashlightConeDegrees, 68.0f, 78.0f);
        settings_.airParticles = true;
        settings_.airParticleDensity = std::max(0.32f, settings_.airParticleDensity * 0.55f);
        settings_.sparkParticles = true;
        settings_.fadeInSeconds = std::max(settings_.fadeInSeconds, 1.85f);
        settings_.bloodWorldCoverage = std::max(settings_.bloodWorldCoverage, 0.45f);
        settings_.bloodWorldAlwaysOn = false;
        settings_.bloodWorldFlickerIntensity = 0.0f;
        settings_.fogStartMeters = 2.6f;
        settings_.fogEndMeters = 6.4f;
    }

    MenuPlaquePlacement MenuButtonPlacement(int index) const {
        XMFLOAT3 c = maze_.WorldCenter(maze_.start, 0.0f);
        const float northWallZ = c.z + maze_.tileD * 0.5f - 0.034f;
        MenuPlaquePlacement plaque{};
        plaque.halfW = std::min(maze_.tileW * 0.68f, 1.10f);
        plaque.halfH = 0.150f;
        plaque.center = {c.x + maze_.tileW * 0.50f, 1.72f - static_cast<float>(index) * 0.36f, northWallZ};
        plaque.right = {1.0f, 0.0f, 0.0f};
        plaque.inward = {0.0f, 0.0f, -1.0f};
        return plaque;
    }

    bool ProjectMenuQuadToScreen(XMFLOAT3 center, XMFLOAT3 right, XMFLOAT3 up, float halfW, float halfH, RECT& out) const {
        if (width_ <= 0 || height_ <= 0) return false;
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});

        XMFLOAT3 f = Forward();
        XMVECTOR eye = XMLoadFloat3(&camera_);
        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        XMVECTOR viewDir = XMVector3Normalize(XMVectorSet(f.x, lookPitch_, f.z, 0.0f));
        XMMATRIX view = XMMatrixLookAtLH(eye, eye + viewDir, worldUp);
        float aspect = static_cast<float>(std::max<LONG>(1, width_)) / static_cast<float>(std::max<LONG>(1, height_));
        float fovDegrees = runtimeMode_ == RendererRuntimeMode::MainMenu ? 84.0f : 70.0f;
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovDegrees * kPi / 180.0f, aspect, 0.045f, 42.0f);
        XMMATRIX viewProj = view * proj;

        auto p = [&](float x, float y) {
            return Add3(center, Add3(Scale3(right, x * halfW), Scale3(up, y * halfH)));
        };
        std::array<XMFLOAT3, 4> corners = {{
            p(-1.0f, -1.0f),
            p( 1.0f, -1.0f),
            p( 1.0f,  1.0f),
            p(-1.0f,  1.0f)
        }};

        float minX = static_cast<float>(width_);
        float minY = static_cast<float>(height_);
        float maxX = 0.0f;
        float maxY = 0.0f;
        int visible = 0;
        for (const XMFLOAT3& corner : corners) {
            XMVECTOR clip = XMVector3TransformCoord(XMLoadFloat3(&corner), viewProj);
            XMFLOAT3 ndc{};
            XMStoreFloat3(&ndc, clip);
            if (ndc.z < 0.0f || ndc.z > 1.0f) continue;
            float sx = (ndc.x * 0.5f + 0.5f) * static_cast<float>(width_);
            float sy = (0.5f - ndc.y * 0.5f) * static_cast<float>(height_);
            minX = std::min(minX, sx);
            minY = std::min(minY, sy);
            maxX = std::max(maxX, sx);
            maxY = std::max(maxY, sy);
            ++visible;
        }
        if (visible < 3) return false;

        constexpr LONG pad = 4;
        out.left = std::clamp(static_cast<LONG>(std::floor(minX)) - pad, 0L, static_cast<LONG>(width_));
        out.top = std::clamp(static_cast<LONG>(std::floor(minY)) - pad, 0L, static_cast<LONG>(height_));
        out.right = std::clamp(static_cast<LONG>(std::ceil(maxX)) + pad, 0L, static_cast<LONG>(width_));
        out.bottom = std::clamp(static_cast<LONG>(std::ceil(maxY)) + pad, 0L, static_cast<LONG>(height_));
        return out.right > out.left && out.bottom > out.top;
    }
