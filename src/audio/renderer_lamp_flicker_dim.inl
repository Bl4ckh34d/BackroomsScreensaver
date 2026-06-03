    bool RuntimeLampFlickerDim(const RuntimeLampState& lamp) const {
        float cellX = static_cast<float>(lamp.tile.x);
        float cellZ = static_cast<float>(lamp.tile.y);
        bool flickerFixture = LampHash(cellX + 17.0f, cellZ + 17.0f) >= 1.0f - settingsRuntime_.live.lampFlickerRatio;
        if (!flickerFixture) return false;
        float h = LampHash(cellX, cellZ);
        float tick = std::floor(timeRuntime_.time * (1.3f + LampHash(cellX + 37.0f, cellZ + 37.0f) * 2.5f));
        bool event = LampHash(cellX + tick + 71.0f, cellZ + tick + 71.0f) >= 0.86f;
        if (!event) return false;
        float flutter = 0.18f + 0.82f * Clamp01(std::sin(timeRuntime_.time * (41.0f + h * 50.0f)) * 0.5f + 0.5f);
        return flutter < 0.42f;
    }
