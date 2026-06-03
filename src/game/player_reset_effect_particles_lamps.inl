        effectRuntime_.airParticles.clear();
        effectRuntime_.airParticles.reserve(22000);
        effectRuntime_.airParticleBudgetScale = 1.0f;
        effectRuntime_.airParticleFrameDt = 0.0f;
        effectRuntime_.airParticleValidationCursor = 0;
        viewRuntime_.propLookTimer = 0.0f;
        viewRuntime_.propLookDuration = 0.0f;
        viewRuntime_.propLookCooldown = RandRange(0.7f, 2.0f);
        viewRuntime_.propLookScanSeed = RandRange(0.0f, 1.0f);
        viewRuntime_.lastPropLookTarget = {};
        viewRuntime_.hasLastPropLookTarget = false;
        effectRuntime_.sparks.clear();
        effectRuntime_.sparkFlashes.clear();
        effectRuntime_.sparkChains.clear();
        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            lamp.damage = 0.0f;
            lamp.sparkTimer = RandRange(0.08f, 0.72f);
            lamp.broken = false;
            lamp.flickerWasDim = false;
            lamp.flickerClickCooldown = 0.0f;
        }
        if (!effectRuntime_.lampDamagePixels.empty()) {
            std::fill(effectRuntime_.lampDamagePixels.begin(), effectRuntime_.lampDamagePixels.end(), 0);
            effectRuntime_.lampDamageDirty = true;
        }
        effectRuntime_.steam.clear();
