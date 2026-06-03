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
