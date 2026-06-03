    bool BloodWorldWetFootstepsActive() const {
        if (settingsRuntime_.live.bloodWorldCoverage <= 0.001f) return false;
        if (settingsRuntime_.live.bloodWorldAlwaysOn && settingsRuntime_.live.bloodWorldFlickerIntensity > 0.001f) return true;
        if (scareRuntime_.bloodWorldFlickerTimer > 0.0f && scareRuntime_.bloodWorldFlickerDuration > 0.001f &&
            settingsRuntime_.live.bloodWorldFlickerIntensity > 0.001f) {
            float elapsed = scareRuntime_.bloodWorldFlickerDuration - scareRuntime_.bloodWorldFlickerTimer;
            float envelope = SmoothStep(0.0f, 0.055f, elapsed) *
                (1.0f - SmoothStep(scareRuntime_.bloodWorldFlickerDuration - 0.18f, scareRuntime_.bloodWorldFlickerDuration, elapsed));
            float strobe = ((std::sin(elapsed * 41.0f) + std::sin(elapsed * 93.0f) * 0.48f +
                std::sin(elapsed * 151.0f) * 0.22f) > -0.06f) ? 1.0f : 0.0f;
            return envelope * strobe * settingsRuntime_.live.bloodWorldFlickerIntensity > 0.001f;
        }
        return false;
    }

    bool IsWetFootstepAtPlayer() const {
        if (BloodWorldWetFootstepsActive()) return true;

        float radius = std::max(0.28f, gameWorld_.maze.TileMinimum() * 0.22f);
        XMFLOAT3 forward{std::sin(gameWorld_.player.yaw), 0.0f, std::cos(gameWorld_.player.yaw)};
        XMFLOAT3 right{forward.z, 0.0f, -forward.x};
        const XMFLOAT2 samples[] = {
            {0.0f, 0.0f},
            { right.x * radius, right.z * radius },
            {-right.x * radius, -right.z * radius },
            { forward.x * radius, forward.z * radius },
            {-forward.x * radius, -forward.z * radius }
        };
        for (const XMFLOAT2& sample : samples) {
            if (IsWetFootstepPoint(gameWorld_.player.position.x + sample.x, gameWorld_.player.position.z + sample.y)) {
                return true;
            }
        }
        return false;
    }
