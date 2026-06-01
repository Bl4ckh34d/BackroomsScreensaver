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
        settings_.lampIntensity = std::max(settings_.lampIntensity, 1.85f);
        settings_.lampFlickerRatio = 0.080f;
        settings_.brokenZoneRatio = 0.0f;
        settings_.ambientLight = std::max(settings_.ambientLight, 0.045f);
        settings_.exposure = std::max(settings_.exposure, 1.12f);
        settings_.flashlightIntensity = std::clamp(settings_.flashlightIntensity, 1.15f, 1.55f);
        settings_.flashlightAttenuation = std::min(settings_.flashlightAttenuation, 0.048f);
        settings_.flashlightConeDegrees = std::clamp(settings_.flashlightConeDegrees, 68.0f, 78.0f);
        settings_.airParticles = true;
        settings_.airParticleDensity = std::max(0.32f, settings_.airParticleDensity * 0.55f);
        settings_.sparkParticles = true;
        settings_.fadeInSeconds = std::max(settings_.fadeInSeconds, 1.85f);
        settings_.bloodWorldCoverage = std::max(settings_.bloodWorldCoverage, 0.45f);
        settings_.bloodWorldAlwaysOn = false;
        settings_.bloodWorldFlickerIntensity = 0.0f;
    }

    MenuPlaquePlacement MenuButtonPlacement(int index) const {
        XMFLOAT3 c = maze_.WorldCenter(maze_.start, 0.0f);
        const float northWallZ = c.z + maze_.tileD * 0.5f - 0.034f;
        MenuPlaquePlacement plaque{};
        plaque.halfW = std::min(maze_.tileW * 0.68f, 1.10f);
        int count = std::clamp(menuButtonCount_, 3, static_cast<int>(menuButtonLabelRows_.size()));
        float spacing = count >= 6 ? 0.300f : (count >= 5 ? 0.325f : 0.360f);
        float topY = count >= 6 ? 1.90f : (count >= 5 ? 1.82f : 1.72f);
        plaque.halfH = count >= 5 ? 0.132f : 0.150f;
        plaque.center = {c.x + maze_.tileW * 0.50f, topY - static_cast<float>(index) * spacing, northWallZ};
        plaque.right = {1.0f, 0.0f, 0.0f};
        plaque.inward = {0.0f, 0.0f, -1.0f};
        return plaque;
    }

    MenuPlaquePlacement MenuCustomPanelPlacement() const {
        XMFLOAT3 c = maze_.WorldCenter(maze_.start, 0.0f);
        float eastWallX = c.x + maze_.tileW * 2.5f - 0.034f;
        MenuPlaquePlacement plaque{};
        plaque.halfW = std::min(maze_.tileD * 0.78f, 1.25f);
        plaque.halfH = std::min(maze_.tileD * 0.48f, 0.76f);
        plaque.center = {eastWallX, 1.54f, c.z - maze_.tileD * 0.80f};
        plaque.right = {0.0f, 0.0f, -1.0f};
        plaque.inward = {-1.0f, 0.0f, 0.0f};
        return plaque;
    }

    void MainMenuCustomCameraPose(XMFLOAT3& outCamera, float& outYaw, float& outPitch) const {
        MenuPlaquePlacement panel = MenuCustomPanelPlacement();
        XMFLOAT3 target = Add3(panel.center, {0.0f, -0.02f, 0.0f});
        outCamera = Add3(panel.center, Add3(Scale3(panel.inward, 2.00f), {0.0f, 0.08f, 0.06f}));
        outYaw = YawToPoint(target);
        outPitch = std::clamp(PitchToPoint(target), -0.42f, 0.38f);
    }

    bool MainMenuCustomPanelLampTile(Tile lampTile) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return false;
        const int panelX = std::clamp(maze_.start.x + 2, 1, maze_.w - 2);
        return lampTile.x == panelX && (lampTile.y == maze_.start.y || lampTile.y == maze_.start.y - 1);
    }

    bool MainMenuExitLampTile(Tile lampTile) const {
        return runtimeMode_ == RendererRuntimeMode::MainMenu && lampTile == maze_.start;
    }

    bool MainMenuPrimaryLampTile(Tile lampTile) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return false;
        const int primaryX = std::clamp(maze_.start.x + 1, 1, maze_.w - 2);
        return lampTile.x == primaryX && lampTile.y == maze_.start.y;
    }

    bool MainMenuAlwaysLitLampTile(Tile lampTile) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return true;
        const int hallX = std::clamp(maze_.start.x + 1, 1, maze_.w - 2);
        const int hallEndY = std::max(1, maze_.start.y - 2);
        return lampTile.x == hallX && lampTile.y >= 1 && lampTile.y <= hallEndY;
    }

    bool MainMenuAllowedLampTile(Tile lampTile) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return true;
        return MainMenuExitLampTile(lampTile) || MainMenuAlwaysLitLampTile(lampTile) ||
            MainMenuPrimaryLampTile(lampTile);
    }

    bool MainMenuLampShouldBeLit(Tile lampTile) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return true;
        if (MainMenuExitLampTile(lampTile)) return !(menuDarkLayerOneRun_ && menuLampBurstPlayed_);
        if (MainMenuAlwaysLitLampTile(lampTile)) return true;
        if (menuStartTransitionActive_ || menuStartTransitionComplete_) return false;
        if (MainMenuPrimaryLampTile(lampTile)) return !menuCustomViewTarget_;
        if (MainMenuCustomPanelLampTile(lampTile)) return false;
        return false;
    }

    void UpdateMainMenuLampOverrides() {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu || lampDamagePixels_.empty()) return;
        bool changed = false;
        for (int y = 0; y < maze_.h; ++y) {
            for (int x = 0; x < maze_.w; ++x) {
                Tile lampTile{x, y};
                if (!MainMenuAllowedLampTile(lampTile)) continue;
                size_t index = static_cast<size_t>(y * maze_.w + x);
                if (index >= lampDamagePixels_.size()) continue;
                uint8_t desired = MainMenuLampShouldBeLit(lampTile) ? 0 : 255;
                if (lampDamagePixels_[index] != desired) {
                    lampDamagePixels_[index] = desired;
                    changed = true;
                }
            }
        }
        for (RuntimeLampState& lamp : runtimeLamps_) {
            if (!MainMenuAllowedLampTile(lamp.tile)) continue;
            bool lit = MainMenuLampShouldBeLit(lamp.tile);
            lamp.broken = !lit;
            lamp.damage = lit ? 0.0f : 1.0f;
        }
        if (changed) lampDamageDirty_ = true;
    }

    static bool CustomGameControlPixelRect(CustomGameMenuControl control, RECT& r) {
        switch (control) {
        case CustomGameMenuControl::BrokenLampScares: r = {36, 126, 250, 152}; return true;
        case CustomGameMenuControl::AirVentScares: r = {36, 160, 250, 186}; return true;
        case CustomGameMenuControl::WaterScares: r = {36, 194, 250, 220}; return true;
        case CustomGameMenuControl::BloodWorldScares: r = {270, 126, 492, 152}; return true;
        case CustomGameMenuControl::FleshWorldScares: r = {270, 160, 492, 186}; return true;
        case CustomGameMenuControl::BrokenLampScareDetails: r = {222, 126, 250, 152}; return true;
        case CustomGameMenuControl::AirVentScareDetails: r = {222, 160, 250, 186}; return true;
        case CustomGameMenuControl::WaterScareDetails: r = {222, 194, 250, 220}; return true;
        case CustomGameMenuControl::BloodWorldScareDetails: r = {464, 126, 492, 152}; return true;
        case CustomGameMenuControl::FleshWorldScareDetails: r = {464, 160, 492, 186}; return true;
        case CustomGameMenuControl::OmukadeBoss: r = {36, 254, 250, 280}; return true;
        case CustomGameMenuControl::EightPages: r = {270, 254, 492, 280}; return true;
        case CustomGameMenuControl::SizeXMinus: r = {172, 320, 200, 348}; return true;
        case CustomGameMenuControl::SizeXPlus: r = {270, 320, 298, 348}; return true;
        case CustomGameMenuControl::SizeYMinus: r = {172, 354, 200, 382}; return true;
        case CustomGameMenuControl::SizeYPlus: r = {270, 354, 298, 382}; return true;
        case CustomGameMenuControl::RoomCountMinus: r = {172, 388, 200, 416}; return true;
        case CustomGameMenuControl::RoomCountPlus: r = {270, 388, 298, 416}; return true;
        case CustomGameMenuControl::EnvironmentDetails: r = {310, 286, 492, 314}; return true;
        case CustomGameMenuControl::EnvDirtMinus: r = {180, 116, 208, 144}; return true;
        case CustomGameMenuControl::EnvDirtPlus: r = {306, 116, 334, 144}; return true;
        case CustomGameMenuControl::EnvPaperMinus: r = {180, 146, 208, 174}; return true;
        case CustomGameMenuControl::EnvPaperPlus: r = {306, 146, 334, 174}; return true;
        case CustomGameMenuControl::EnvPropMinus: r = {180, 176, 208, 204}; return true;
        case CustomGameMenuControl::EnvPropPlus: r = {306, 176, 334, 204}; return true;
        case CustomGameMenuControl::EnvLampOnMinus: r = {180, 216, 208, 244}; return true;
        case CustomGameMenuControl::EnvLampOnPlus: r = {306, 216, 334, 244}; return true;
        case CustomGameMenuControl::EnvLampFlickerMinus: r = {180, 246, 208, 274}; return true;
        case CustomGameMenuControl::EnvLampFlickerPlus: r = {306, 246, 334, 274}; return true;
        case CustomGameMenuControl::EnvLampSparkMinus: r = {180, 276, 208, 304}; return true;
        case CustomGameMenuControl::EnvLampSparkPlus: r = {306, 276, 334, 304}; return true;
        case CustomGameMenuControl::EnvFogStartMinus: r = {180, 316, 208, 344}; return true;
        case CustomGameMenuControl::EnvFogStartPlus: r = {306, 316, 334, 344}; return true;
        case CustomGameMenuControl::EnvFogEndMinus: r = {180, 346, 208, 374}; return true;
        case CustomGameMenuControl::EnvFogEndPlus: r = {306, 346, 334, 374}; return true;
        case CustomGameMenuControl::EnvFogDarkMinus: r = {180, 376, 208, 404}; return true;
        case CustomGameMenuControl::EnvFogDarkPlus: r = {306, 376, 334, 404}; return true;
        case CustomGameMenuControl::ScareChanceMinus: r = {180, 220, 208, 248}; return true;
        case CustomGameMenuControl::ScareChancePlus: r = {306, 220, 334, 248}; return true;
        case CustomGameMenuControl::ScareStartMinMinus: r = {180, 270, 208, 298}; return true;
        case CustomGameMenuControl::ScareStartMinPlus: r = {306, 270, 334, 298}; return true;
        case CustomGameMenuControl::ScareStartMaxMinus: r = {180, 320, 208, 348}; return true;
        case CustomGameMenuControl::ScareStartMaxPlus: r = {306, 320, 334, 348}; return true;
        case CustomGameMenuControl::ScareDetailBack: r = {198, 430, 314, 462}; return true;
        case CustomGameMenuControl::Start: r = {342, 430, 456, 462}; return true;
        case CustomGameMenuControl::Back: r = {56, 430, 170, 462}; return true;
        default: return false;
        }
    }

    bool CustomGameControlPlacement(CustomGameMenuControl control, MenuPlaquePlacement& out) const {
        RECT r{};
        if (!CustomGameControlPixelRect(control, r)) return false;
        MenuPlaquePlacement panel = MenuCustomPanelPlacement();
        constexpr float logicalSize = 512.0f;
        float centerX = ((static_cast<float>(r.left + r.right) * 0.5f) / logicalSize - 0.5f) * panel.halfW * 2.0f;
        float centerY = (0.5f - (static_cast<float>(r.top + r.bottom) * 0.5f) / logicalSize) * panel.halfH * 2.0f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        out = panel;
        out.center = Add3(panel.center, Add3(Add3(Scale3(panel.right, centerX), Scale3(up, centerY)), Scale3(panel.inward, 0.082f)));
        out.halfW = (static_cast<float>(r.right - r.left) / logicalSize) * panel.halfW;
        out.halfH = (static_cast<float>(r.bottom - r.top) / logicalSize) * panel.halfH;
        return out.halfW > 0.0f && out.halfH > 0.0f;
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
