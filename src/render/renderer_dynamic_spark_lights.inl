    std::array<XMFLOAT4, 2> ActiveSparkLights() const {
        std::array<XMFLOAT4, 2> lights{};
        std::array<float, 2> power{};
        for (const SparkFlash& flash : effectRuntime_.sparkFlashes) {
            float lifeLeft = Clamp01(1.0f - flash.age / std::max(0.001f, flash.life));
            float p = flash.intensity * lifeLeft * lifeLeft;
            if (p <= 0.02f) continue;
            int slot = power[0] <= power[1] ? 0 : 1;
            if (p > power[static_cast<size_t>(slot)]) {
                power[static_cast<size_t>(slot)] = p;
                lights[static_cast<size_t>(slot)] = {flash.pos.x, flash.pos.y, flash.pos.z, p};
            }
        }
        return lights;
    }
