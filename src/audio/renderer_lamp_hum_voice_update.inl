        for (size_t i = 0; i < effectRuntime_.runtimeLamps.size(); ++i) {
            int tag = LampHumVoiceTag(i);
            const RuntimeLampState& lamp = effectRuntime_.runtimeLamps[i];
            float dx = lamp.pos.x - gameWorld_.player.position.x;
            float dy = lamp.pos.y - gameWorld_.player.position.y;
            float dz = lamp.pos.z - gameWorld_.player.position.z;
            bool tooFar = dx * dx + dy * dy + dz * dz > stopDistSq;
            if (lamp.broken || tooFar || RuntimeLampFlickerDim(lamp) || i >= shouldPlay.size() || !shouldPlay[i]) {
                audioRuntime_.engine.StopTaggedVoice(tag);
                continue;
            }
            if (audioRuntime_.engine.HasTaggedVoice(tag)) continue;

            GameSound hum = GameSound::NeonHumQuiet;
            if (lamp.humVariant == 1) hum = GameSound::NeonHumLoud;
            if (lamp.humVariant == 2) hum = GameSound::NeonHumLoud2;
            float baseVolume = 0.045f * 1.15f * 0.80f;
            float lampVariation = Lerp(0.84f, 1.18f,
                Rand01(lamp.tile.x * 19 + 17, lamp.tile.y * 29 + 31, sessionRuntime_.runtimeSeed ^ 0xA04D10u));
            float damageLift = Lerp(0.94f, 1.10f, Clamp01(lamp.damage));
            float volume = baseVolume * lampVariation * damageLift;
            uint32_t tx = static_cast<uint32_t>(lamp.tile.x + 4096);
            uint32_t ty = static_cast<uint32_t>(lamp.tile.y + 4096);
            uint32_t stableHumId = sessionRuntime_.runtimeSeed ^ (tx * 73856093u) ^ (ty * 19349663u) ^
                (static_cast<uint32_t>(lamp.humVariant + 17) * 83492791u);
            size_t humSample = audioRuntime_.engine.PickStableSample(hum, stableHumId);
            audioRuntime_.engine.StartLoopTaggedSample(hum, humSample, AudioBus::Ambience, lamp.pos, volume, true, tag, AudioOcclusionFor(lamp.pos));
        }
    }
