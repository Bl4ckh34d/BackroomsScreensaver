    float AirParticleLevelDensityScale() const {
        if (!IsPlayableSimulationMode(sessionRuntime_.mode)) return 1.0f;
        return gameWorld_.AirParticleDensityScale();
    }

    int DesiredAirParticleCount() const {
        if (!settingsRuntime_.live.airParticles || settingsRuntime_.live.airParticleDensity <= 0.001f || monsterPreview_.active) return 0;
        float density = std::clamp(settingsRuntime_.live.airParticleDensity, 0.0f, 4.0f) * AirParticleLevelDensityScale();
        float budget = std::clamp(effectRuntime_.airParticleBudgetScale, 0.34f, 1.0f);
        return std::clamp(static_cast<int>(std::round(2800.0f * density * budget)), 0, 9000);
    }
