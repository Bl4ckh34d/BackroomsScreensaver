// Spark, steam, and vent-drop particle simulation updates. 
// Included inside Renderer's private section from scare_effect_events.inl.

    void UpdateSparks(float dt) {
        if (!settingsRuntime_.live.sparkParticles) {
            effectRuntime_.sparks.clear();
            effectRuntime_.sparkFlashes.clear();
            effectRuntime_.sparkChains.clear();
            return;
        }
        for (SparkChain& chain : effectRuntime_.sparkChains) {
            chain.timer -= dt;
            while (chain.remaining > 0 && chain.timer <= 0.0f) {
                EmitSparkBurstAt(chain.pos, chain.intensity * RandRange(0.68f, 1.18f));
                --chain.remaining;
                chain.timer += RandRange(0.075f, 0.22f);
            }
        }
        effectRuntime_.sparkChains.erase(std::remove_if(effectRuntime_.sparkChains.begin(), effectRuntime_.sparkChains.end(), [](const SparkChain& chain) {
            return chain.remaining <= 0;
        }), effectRuntime_.sparkChains.end());

        effectRuntime_.sparkCooldown = 1000000.0f;

        for (SparkParticle& sp : effectRuntime_.sparks) {
            sp.age += dt;
            sp.vel.y -= 3.8f * dt;
            sp.pos.x += sp.vel.x * dt;
            sp.pos.y += sp.vel.y * dt;
            sp.pos.z += sp.vel.z * dt;
            if (sp.pos.y < 0.055f && sp.vel.y < 0.0f) {
                sp.pos.y = 0.055f;
                sp.vel.y = -sp.vel.y * 0.34f;
                sp.vel.x *= 0.58f;
                sp.vel.z *= 0.58f;
                if (std::abs(sp.vel.y) < 0.08f) sp.age = sp.life;
            }
        }
        effectRuntime_.sparks.erase(std::remove_if(effectRuntime_.sparks.begin(), effectRuntime_.sparks.end(), [](const SparkParticle& sp) {
            return sp.age >= sp.life;
        }), effectRuntime_.sparks.end());

        for (SparkFlash& flash : effectRuntime_.sparkFlashes) {
            flash.age += dt;
        }
        effectRuntime_.sparkFlashes.erase(std::remove_if(effectRuntime_.sparkFlashes.begin(), effectRuntime_.sparkFlashes.end(), [](const SparkFlash& flash) {
            return flash.age >= flash.life;
        }), effectRuntime_.sparkFlashes.end());
    }
