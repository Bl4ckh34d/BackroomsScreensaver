                if (lampOn) {
                    effectRuntime_.runtimeLamps.push_back({
                        lampTile,
                        {lampCenter.x, ctx.wallH - 0.08f, lampCenter.z},
                        0.0f,
                        RandRange(0.08f, 0.72f),
                        false,
                        [&]() {
                            float humRoll = LampHash(static_cast<float>(cellX) + 14.7f, static_cast<float>(cellZ) - 42.3f);
                            if (humRoll < 0.05f) return 2;
                            if (humRoll < 0.15f) return 1;
                            return 0;
                        }()
                    });
                    if (jumpscareLamp && settingsRuntime_.live.sparkParticles) {
                        effectRuntime_.sparkEmitters.push_back({{lampCenter.x, ctx.wallH - 0.085f, lampCenter.z}});
                    }
                } else if (((sessionRuntime_.mode == RendererRuntimeMode::MainMenu && brokenPanel) || wetLampTile) &&
                           brokenPanel && settingsRuntime_.live.sparkParticles) {
                    SparkEmitter emitter{{lampCenter.x, ctx.wallH - 0.085f, lampCenter.z}};
                    if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
                        emitter.cooldown = 1.4f + LampHash(static_cast<float>(cellX) + 51.2f, static_cast<float>(cellZ) - 17.4f) * 4.8f;
                    }
                    effectRuntime_.sparkEmitters.push_back(emitter);
                }
