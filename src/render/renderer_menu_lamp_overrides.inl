    void UpdateMainMenuLampOverrides() {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu || effectRuntime_.lampDamagePixels.empty()) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const Maze& maze = *world.maze;
        bool changed = false;
        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                Tile lampTile{x, y};
                if (!MainMenuAllowedLampTile(lampTile)) continue;
                size_t index = static_cast<size_t>(y * maze.w + x);
                if (index >= effectRuntime_.lampDamagePixels.size()) continue;
                uint8_t desired = MainMenuLampDamageValue(lampTile);
                if (effectRuntime_.lampDamagePixels[index] != desired) {
                    effectRuntime_.lampDamagePixels[index] = desired;
                    changed = true;
                }
            }
        }
        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            if (!MainMenuAllowedLampTile(lamp.tile)) continue;
            uint8_t damageValue = MainMenuLampDamageValue(lamp.tile);
            lamp.damage = static_cast<float>(damageValue) / 255.0f;
            lamp.broken = lamp.damage >= 0.995f;
        }
        if (changed) effectRuntime_.lampDamageDirty = true;
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
