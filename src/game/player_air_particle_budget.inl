    void UpdateAirParticlePerformanceBudget(float dt) {
        if (!settingsRuntime_.live.airParticles || monsterPreview_.active) {
            effectRuntime_.airParticleBudgetScale = 1.0f;
            effectRuntime_.airParticleFrameDt = 0.0f;
            return;
        }
        if (dt <= 0.0f || dt >= 0.20f) return;
        if (effectRuntime_.airParticleFrameDt <= 0.0f) effectRuntime_.airParticleFrameDt = dt;
        effectRuntime_.airParticleFrameDt += (dt - effectRuntime_.airParticleFrameDt) * std::min(1.0f, dt * 2.0f);

        float target = 1.0f;
        if (timeRuntime_.time > 2.0f) {
            if (effectRuntime_.airParticleFrameDt > 0.034f) target = 0.40f;
            else if (effectRuntime_.airParticleFrameDt > 0.028f) target = 0.55f;
            else if (effectRuntime_.airParticleFrameDt > 0.023f) target = 0.72f;
            else if (effectRuntime_.airParticleFrameDt > 0.019f) target = 0.88f;
        }
        float follow = target < effectRuntime_.airParticleBudgetScale
            ? std::min(1.0f, dt * 3.0f)
            : std::min(1.0f, dt * 0.22f);
        effectRuntime_.airParticleBudgetScale += (target - effectRuntime_.airParticleBudgetScale) * follow;
        effectRuntime_.airParticleBudgetScale = std::clamp(effectRuntime_.airParticleBudgetScale, 0.34f, 1.0f);
    }
