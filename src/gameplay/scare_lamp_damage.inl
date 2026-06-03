    void MarkLampDamagePixel(Tile tile, float damage) {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y) || effectRuntime_.lampDamagePixels.empty()) return;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        if (index >= effectRuntime_.lampDamagePixels.size()) return;
        uint8_t value = Byte(damage);
        if (value > effectRuntime_.lampDamagePixels[index]) {
            effectRuntime_.lampDamagePixels[index] = value;
            effectRuntime_.lampDamageDirty = true;
        }
    }

    bool LampCanEmitSparks(const RuntimeLampState& lamp) const {
        return IsWetCeilingTile(lamp.tile) || IsWetFootstepTile(lamp.tile);
    }

    void BreakRuntimeLamp(RuntimeLampState& lamp, bool emitPlayerNoise = true, bool playBreakSound = true) {
        if (lamp.broken) return;
        lamp.broken = true;
        lamp.damage = 1.0f;
        lamp.sparkTimer = RandRange(0.55f, 1.85f);
        MarkLampDamagePixel(lamp.tile, lamp.damage);
        if (playBreakSound) QueueLightBulbBreakSoundAt(lamp.pos, 1.25f, emitPlayerNoise);
        if (settingsRuntime_.live.sparkParticles) {
            float intensity = std::max(2.8f, PickBrokenLampSparkIntensity() * 1.45f);
            EmitSparkBurstAt(lamp.pos, intensity);
            if (LampCanEmitSparks(lamp) || RandRange(0.0f, 1.0f) < 0.35f) {
                ScheduleSparkChain(lamp.pos, intensity * settingsRuntime_.live.effectBrokenLampChainIntensityScale,
                    std::max(3, PickBrokenLampChainBursts() + 2));
            }
        }
    }

    bool BreakNearestRuntimeLampAt(const XMFLOAT3& pos, float maxDistance) {
        RuntimeLampState* nearest = nullptr;
        float bestSq = maxDistance * maxDistance;
        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            if (lamp.broken) continue;
            float dx = lamp.pos.x - pos.x;
            float dy = lamp.pos.y - pos.y;
            float dz = lamp.pos.z - pos.z;
            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq <= bestSq) {
                bestSq = distSq;
                nearest = &lamp;
            }
        }
        if (!nearest) return false;
        BreakRuntimeLamp(*nearest);
        return true;
    }
