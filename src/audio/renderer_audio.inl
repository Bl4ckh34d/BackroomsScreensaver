    XMFLOAT3 MonsterSoundOrigin() const {
        return MonsterEyeFocus();
    }

    void MarkWetFootstepTile(Tile tile) {
        if (!maze_.InBounds(tile.x, tile.y)) return;
        size_t index = static_cast<size_t>(tile.y * maze_.w + tile.x);
        if (index < wetFootstepTiles_.size()) wetFootstepTiles_[index] = 1;
    }

    void MarkWetFootstepArea(float px, float pz, float width, float depth, float yaw, float extra = 0.06f, float wetDelaySeconds = 0.0f) {
        WetFloorFootprint fp{};
        fp.center = {px, pz};
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        fp.right = {c, -s};
        fp.forward = {s, c};
        fp.halfW = std::max(0.01f, width * 0.5f + extra);
        fp.halfD = std::max(0.01f, depth * 0.5f + extra);
        fp.wetDelaySeconds = std::max(0.0f, wetDelaySeconds);
        wetFloorFootprints_.push_back(fp);
        if (wetFloorFootprints_.size() > 4096) {
            wetFloorFootprints_.erase(wetFloorFootprints_.begin(), wetFloorFootprints_.begin() + 512);
        }

        auto mark = [&](float lx, float lz) {
            float x = px + fp.right.x * lx + fp.forward.x * lz;
            float z = pz + fp.right.y * lx + fp.forward.y * lz;
            MarkWetFootstepTile(maze_.TileFromWorld(x, z));
        };
        MarkWetFootstepTile(maze_.TileFromWorld(px, pz));
        mark(-fp.halfW, -fp.halfD);
        mark( fp.halfW, -fp.halfD);
        mark(-fp.halfW,  fp.halfD);
        mark( fp.halfW,  fp.halfD);
    }

    bool IsWetFootstepTile(Tile tile) const {
        if (!maze_.InBounds(tile.x, tile.y)) return false;
        size_t index = static_cast<size_t>(tile.y * maze_.w + tile.x);
        return index < wetFootstepTiles_.size() && wetFootstepTiles_[index] != 0;
    }

    bool IsNearWetFootstepTile(float x, float z) const {
        Tile center = maze_.TileFromWorld(x, z);
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (IsWetFootstepTile({center.x + dx, center.y + dy})) return true;
            }
        }
        return false;
    }

    bool MatureWaterRevealContains(float x, float z, float radius = 0.0f) const {
        for (const BloodRevealRegion& region : bloodRevealRegions_) {
            if (!region.waterLiquid || region.radius <= 0.01f || region.activationTime <= -999000.0f) continue;
            float age = time_ - region.activationTime;
            if (age < 5.8f) continue;
            float grow = SmoothStep(5.8f, 24.0f, age);
            float activeRadius = Lerp(std::min(region.radius, 0.42f), region.radius * 0.86f, grow);
            float dx = x - region.center.x;
            float dz = z - region.center.z;
            float effectiveRadius = activeRadius + radius;
            if (dx * dx + dz * dz <= effectiveRadius * effectiveRadius) {
                return true;
            }
        }
        return false;
    }

    bool IsWetFootstepPoint(float x, float z, float radius = 0.08f) const {
        if (!MatureWaterRevealContains(x, z, radius)) return false;
        for (const WetFloorFootprint& fp : wetFloorFootprints_) {
            if (fp.wetDelaySeconds > 0.0f) {
                bool delayedEnough = false;
                for (const BloodRevealRegion& region : bloodRevealRegions_) {
                    if (!region.waterLiquid || region.activationTime <= -999000.0f) continue;
                    if (time_ - region.activationTime >= fp.wetDelaySeconds) {
                        delayedEnough = true;
                        break;
                    }
                }
                if (!delayedEnough) continue;
            }
            float dx = x - fp.center.x;
            float dz = z - fp.center.y;
            float localX = dx * fp.right.x + dz * fp.right.y;
            float localZ = dx * fp.forward.x + dz * fp.forward.y;
            if (std::abs(localX) <= fp.halfW + radius && std::abs(localZ) <= fp.halfD + radius) {
                return true;
            }
        }
        return false;
    }

    bool BloodWorldWetFootstepsActive() const {
        if (settings_.bloodWorldCoverage <= 0.001f) return false;
        if (settings_.bloodWorldAlwaysOn && settings_.bloodWorldFlickerIntensity > 0.001f) return true;
        if (bloodWorldFlickerTimer_ > 0.0f && bloodWorldFlickerDuration_ > 0.001f &&
            settings_.bloodWorldFlickerIntensity > 0.001f) {
            float elapsed = bloodWorldFlickerDuration_ - bloodWorldFlickerTimer_;
            float envelope = SmoothStep(0.0f, 0.055f, elapsed) *
                (1.0f - SmoothStep(bloodWorldFlickerDuration_ - 0.18f, bloodWorldFlickerDuration_, elapsed));
            float strobe = ((std::sin(elapsed * 41.0f) + std::sin(elapsed * 93.0f) * 0.48f +
                std::sin(elapsed * 151.0f) * 0.22f) > -0.06f) ? 1.0f : 0.0f;
            return envelope * strobe * settings_.bloodWorldFlickerIntensity > 0.001f;
        }
        return false;
    }

    bool IsWetFootstepAtPlayer() const {
        if (BloodWorldWetFootstepsActive()) return true;

        float radius = std::max(0.28f, maze_.TileMinimum() * 0.22f);
        XMFLOAT3 forward{std::sin(yaw_), 0.0f, std::cos(yaw_)};
        XMFLOAT3 right{forward.z, 0.0f, -forward.x};
        const XMFLOAT2 samples[] = {
            {0.0f, 0.0f},
            { right.x * radius, right.z * radius },
            {-right.x * radius, -right.z * radius },
            { forward.x * radius, forward.z * radius },
            {-forward.x * radius, -forward.z * radius }
        };
        for (const XMFLOAT2& sample : samples) {
            if (IsWetFootstepPoint(camera_.x + sample.x, camera_.z + sample.y)) {
                return true;
            }
        }
        return false;
    }

    void MarkWetCeilingTile(Tile tile) {
        if (!maze_.InBounds(tile.x, tile.y)) return;
        size_t index = static_cast<size_t>(tile.y * maze_.w + tile.x);
        if (index < wetCeilingTiles_.size()) wetCeilingTiles_[index] = 1;
    }

    bool IsWetCeilingTile(Tile tile) const {
        if (!maze_.InBounds(tile.x, tile.y)) return false;
        size_t index = static_cast<size_t>(tile.y * maze_.w + tile.x);
        return index < wetCeilingTiles_.size() && wetCeilingTiles_[index] != 0;
    }

    void MarkWetCeilingDripEmitter(XMFLOAT3 pos, float seed) {
        pos.y = 0.10f;
        for (const WetDripEmitter& existing : wetDripEmitters_) {
            float dx = existing.pos.x - pos.x;
            float dz = existing.pos.z - pos.z;
            if (dx * dx + dz * dz < 0.20f) return;
        }
        if (wetDripEmitters_.size() >= 160) return;

        int sx = static_cast<int>(std::floor(pos.x * 41.0f + seed * 97.0f));
        int sz = static_cast<int>(std::floor(pos.z * 43.0f - seed * 83.0f));
        WetDripEmitter emitter{};
        emitter.pos = pos;
        emitter.interval = Lerp(1.0f / 3.0f, 2.0f, Rand01(sx, sz, runtimeSeed_ ^ 0xD21F5u));
        emitter.timer = emitter.interval * Rand01(sx + 19, sz - 31, runtimeSeed_ ^ 0x9E772u);
        emitter.volume = Lerp(0.24f, 0.38f, Rand01(sx - 7, sz + 23, runtimeSeed_ ^ 0x512D9u));
        emitter.age = 0.0f;
        emitter.audibleDelay = Lerp(7.5f, 10.5f, Rand01(sx + 53, sz - 47, runtimeSeed_ ^ 0x71A45u));
        wetDripEmitters_.push_back(emitter);
    }

    void UpdateWetDripAudio(float dt) {
        if (monsterPreview_ || runtimeMode_ == RendererRuntimeMode::MainMenu) return;
        float audibleRange = std::max(18.0f, maze_.TileAverage() * 12.0f);
        for (WetDripEmitter& emitter : wetDripEmitters_) {
            emitter.age += std::max(0.0f, dt);
            if (emitter.age < emitter.audibleDelay) continue;

            emitter.timer -= dt;
            if (emitter.timer > 0.0f) continue;
            if (DistanceToPoint(emitter.pos) <= audibleRange) {
                audio_.PlayRandom(GameSound::WetCarpetCeilingDrip, AudioBus::Effects, emitter.pos,
                    emitter.volume * RandRange(0.90f, 1.10f), true, AudioOcclusionFor(emitter.pos));
            }
            emitter.timer += std::max(1.0f / 3.0f, emitter.interval);
            if (emitter.timer <= 0.0f) emitter.timer = emitter.interval;
        }
    }

    int LampHumVoiceTag(size_t lampIndex) const {
        return 100000 + static_cast<int>(lampIndex);
    }

    bool LampHumAudibleCandidate(const RuntimeLampState& lamp, float maxDistSq) const {
        if (lamp.broken) return false;
        float dx = lamp.pos.x - camera_.x;
        float dy = lamp.pos.y - camera_.y;
        float dz = lamp.pos.z - camera_.z;
        return dx * dx + dy * dy + dz * dz <= maxDistSq;
    }

    void UpdatePersistentLampHumVoices(float dt, bool force = false) {
        if (!audioReady_) return;
        if (!force) {
            lampHumRefreshTimer_ -= std::max(0.0f, dt);
            if (lampHumRefreshTimer_ > 0.0f) return;
        }
        lampHumRefreshTimer_ = force ? 0.0f : 0.22f;

        float maxDist = std::clamp(maze_.TileAverage() * 5.5f, 5.5f, 7.9f);
        float startDistSq = maxDist * maxDist;
        float stopDist = maxDist + std::max(1.75f, maze_.TileAverage() * 0.9f);
        float stopDistSq = stopDist * stopDist;
        constexpr int kMaxActiveLampHums = 24;

        std::vector<LampHumCandidate>& candidates = lampHumCandidates_;
        candidates.clear();
        candidates.reserve(std::min<size_t>(runtimeLamps_.size(), kMaxActiveLampHums * 2));
        for (size_t i = 0; i < runtimeLamps_.size(); ++i) {
            const RuntimeLampState& lamp = runtimeLamps_[i];
            if (!LampHumAudibleCandidate(lamp, startDistSq)) continue;
            if (RuntimeLampFlickerDim(lamp)) continue;
            float dx = lamp.pos.x - camera_.x;
            float dy = lamp.pos.y - camera_.y;
            float dz = lamp.pos.z - camera_.z;
            candidates.push_back({i, dx * dx + dy * dy + dz * dz});
        }
        auto byDistance = [](const LampHumCandidate& a, const LampHumCandidate& b) {
            return a.distSq < b.distSq;
        };
        if (candidates.size() > kMaxActiveLampHums) {
            std::nth_element(candidates.begin(), candidates.begin() + kMaxActiveLampHums, candidates.end(), byDistance);
            candidates.resize(kMaxActiveLampHums);
        }
        std::sort(candidates.begin(), candidates.end(), byDistance);

        lampHumShouldPlay_.assign(runtimeLamps_.size(), 0);
        std::vector<uint8_t>& shouldPlay = lampHumShouldPlay_;
        for (const LampHumCandidate& candidate : candidates) {
            shouldPlay[candidate.index] = 1;
        }

        for (size_t i = 0; i < runtimeLamps_.size(); ++i) {
            int tag = LampHumVoiceTag(i);
            const RuntimeLampState& lamp = runtimeLamps_[i];
            float dx = lamp.pos.x - camera_.x;
            float dy = lamp.pos.y - camera_.y;
            float dz = lamp.pos.z - camera_.z;
            bool tooFar = dx * dx + dy * dy + dz * dz > stopDistSq;
            if (lamp.broken || tooFar || RuntimeLampFlickerDim(lamp) || i >= shouldPlay.size() || !shouldPlay[i]) {
                audio_.StopTaggedVoice(tag);
                continue;
            }
            if (audio_.HasTaggedVoice(tag)) continue;

            GameSound hum = GameSound::NeonHumQuiet;
            if (lamp.humVariant == 1) hum = GameSound::NeonHumLoud;
            if (lamp.humVariant == 2) hum = GameSound::NeonHumLoud2;
            float baseVolume = 0.045f * 1.15f * 0.80f;
            float lampVariation = Lerp(0.84f, 1.18f,
                Rand01(lamp.tile.x * 19 + 17, lamp.tile.y * 29 + 31, runtimeSeed_ ^ 0xA04D10u));
            float damageLift = Lerp(0.94f, 1.10f, Clamp01(lamp.damage));
            float volume = baseVolume * lampVariation * damageLift;
            uint32_t tx = static_cast<uint32_t>(lamp.tile.x + 4096);
            uint32_t ty = static_cast<uint32_t>(lamp.tile.y + 4096);
            uint32_t stableHumId = runtimeSeed_ ^ (tx * 73856093u) ^ (ty * 19349663u) ^
                (static_cast<uint32_t>(lamp.humVariant + 17) * 83492791u);
            size_t humSample = audio_.PickStableSample(hum, stableHumId);
            audio_.StartLoopTaggedSample(hum, humSample, AudioBus::Ambience, lamp.pos, volume, true, tag, AudioOcclusionFor(lamp.pos));
        }
    }

    bool RuntimeLampFlickerDim(const RuntimeLampState& lamp) const {
        float cellX = static_cast<float>(lamp.tile.x);
        float cellZ = static_cast<float>(lamp.tile.y);
        bool flickerFixture = LampHash(cellX + 17.0f, cellZ + 17.0f) >= 1.0f - settings_.lampFlickerRatio;
        if (!flickerFixture) return false;
        float h = LampHash(cellX, cellZ);
        float tick = std::floor(time_ * (1.3f + LampHash(cellX + 37.0f, cellZ + 37.0f) * 2.5f));
        bool event = LampHash(cellX + tick + 71.0f, cellZ + tick + 71.0f) >= 0.86f;
        if (!event) return false;
        float flutter = 0.18f + 0.82f * Clamp01(std::sin(time_ * (41.0f + h * 50.0f)) * 0.5f + 0.5f);
        return flutter < 0.42f;
    }

    void UpdateLampFlickerStarterClicks(float dt) {
        if (!audioReady_ || monsterPreview_ || runtimeLamps_.empty() || settings_.lampFlickerRatio <= 0.001f) return;
        float audibleRange = std::max(5.5f, maze_.TileAverage() * 4.75f);
        float audibleRangeSq = audibleRange * audibleRange;
        for (RuntimeLampState& lamp : runtimeLamps_) {
            lamp.flickerClickCooldown = std::max(0.0f, lamp.flickerClickCooldown - dt);
            if (lamp.broken) {
                lamp.flickerWasDim = false;
                continue;
            }
            float dx = lamp.pos.x - camera_.x;
            float dy = lamp.pos.y - camera_.y;
            float dz = lamp.pos.z - camera_.z;
            bool nearby = dx * dx + dy * dy + dz * dz <= audibleRangeSq;
            bool dim = nearby && RuntimeLampFlickerDim(lamp);
            if (nearby && lamp.flickerWasDim && !dim && lamp.flickerClickCooldown <= 0.0f) {
                PlayNeonFlickerStarterClickAt(lamp.pos);
                lamp.flickerClickCooldown = 0.18f;
            }
            lamp.flickerWasDim = dim;
        }
    }

    void RecomputePlayerNoiseRadiusFromPulses() {
        playerNoiseRadiusMeters_ = 0.0f;
        for (const PlayerAudibleSoundPulse& pulse : playerAudibleSoundPulses_) {
            if (pulse.radius <= 0.0f || pulse.life <= 0.0f || pulse.age >= pulse.life) continue;
            playerNoiseRadiusMeters_ = std::max(playerNoiseRadiusMeters_, pulse.radius);
        }
    }

    void UpdatePlayerAudibleSoundPulses(float dt) {
        for (PlayerAudibleSoundPulse& pulse : playerAudibleSoundPulses_) {
            pulse.age += std::max(0.0f, dt);
        }
        playerAudibleSoundPulses_.erase(std::remove_if(playerAudibleSoundPulses_.begin(), playerAudibleSoundPulses_.end(),
            [](const PlayerAudibleSoundPulse& pulse) {
                return pulse.life <= 0.0f || pulse.age >= pulse.life;
            }), playerAudibleSoundPulses_.end());
        RecomputePlayerNoiseRadiusFromPulses();
    }

    void EmitPlayerAudibleSound(XMFLOAT3 pos, float radius, float life = 0.90f) {
        if (monsterPreview_ || runtimeMode_ == RendererRuntimeMode::MainMenu || radius <= 0.01f) return;
        pos.y = 0.08f;
        PlayerAudibleSoundPulse pulse{};
        pulse.pos = pos;
        pulse.radius = radius;
        pulse.life = std::max(0.10f, life);
        playerAudibleSoundPulses_.push_back(pulse);
        if (playerAudibleSoundPulses_.size() > 18) {
            playerAudibleSoundPulses_.erase(playerAudibleSoundPulses_.begin());
        }
        playerNoiseRadiusMeters_ = std::max(playerNoiseRadiusMeters_, radius);
    }

    void EmitPlayerAudibleSoundAtCamera(float radius, float life = 0.90f) {
        EmitPlayerAudibleSound({camera_.x, 0.08f, camera_.z}, radius, life);
    }

    float TileHearingRadius(float tiles) const {
        return std::max(0.1f, maze_.TileAverage()) * std::max(0.0f, tiles);
    }

    float FootstepHearingRadius(float walkT, float runT, bool crouching, bool wetFootstep) const {
        float radius = 0.0f;
        if (crouching) {
            radius = TileHearingRadius(0.35f);
        } else {
            float runBlend = std::max(runT, runEffort_ * 0.72f);
            if (runBlend > 0.01f) {
                radius = TileHearingRadius(Lerp(3.6f, 4.6f, runBlend));
            } else {
                radius = TileHearingRadius(Lerp(2.1f, 2.9f, walkT));
            }
        }
        return radius * (wetFootstep ? 1.25f : 1.0f);
    }

    float JumpscareHearingRadius(float scale = 1.0f) const {
        return std::max(0.1f, maze_.TileAverage()) * 4.85f * std::max(0.1f, scale);
    }

    float LightBulbBreakHearingRadius() const {
        return TileHearingRadius(20.0f);
    }

    float FlashlightClickHearingRadius() const {
        return std::max(0.55f, maze_.TileMinimum() * 0.62f);
    }

    float AirVentHearingRadius() const {
        return TileHearingRadius(5.0f);
    }

    float SparkHearingRadius(float intensity = 1.0f) const {
        return TileHearingRadius(Lerp(7.0f, 12.0f, Clamp01(intensity / std::max(0.1f, settings_.effectBrokenLampSparkIntensityMax))));
    }

    int FootstepDownBobIndex(float phase) const {
        constexpr float kDownBobPhaseOffset = kPi * 0.75f;
        return static_cast<int>(std::floor((phase - kDownBobPhaseOffset) / kPi));
    }

    void PlayFootstepSound() {
        if (monsterPreview_ || runtimeMode_ == RendererRuntimeMode::MainMenu) return;
        constexpr float kFootstepVolumeScale = 1.80f;
        constexpr float kWetFootstepVolumeScale = 0.25125f;
        XMFLOAT3 pos{camera_.x, 0.08f, camera_.z};
        bool wetFootstep = IsWetFootstepAtPlayer();
        GameSound sound = wetFootstep ? GameSound::SoakedCarpetStep : GameSound::CarpetStep;
        float walkSpeed = std::max(0.1f, settings_.walkSpeed);
        float runSpeed = std::max(walkSpeed + 0.1f, settings_.runSpeed);
        float walkT = Clamp01(smoothedMoveSpeed_ / walkSpeed);
        float runT = Clamp01((smoothedMoveSpeed_ - walkSpeed) / std::max(0.1f, runSpeed - walkSpeed));
        float runAudioBlend = std::max(runT, runEffort_ * 0.72f);
        if (runtimeMode_ == RendererRuntimeMode::PlayableGame && gameInput_.crouch) {
            float volume = Lerp(0.32f, 0.68f, walkT);
            volume *= 0.90f * RandRange(0.88f, 1.06f);
            if (wetFootstep) volume *= kWetFootstepVolumeScale;
            volume *= kFootstepVolumeScale;
            audio_.PlayRandom(sound, AudioBus::Effects, pos, volume, false);
            EmitPlayerAudibleSound(pos, FootstepHearingRadius(walkT, runT, true, wetFootstep), 0.72f);
            return;
        }
        float volume = Lerp(0.74f, 1.14f, walkT);
        volume = Lerp(volume, 2.08f, runAudioBlend);
        volume *= 0.90f * RandRange(0.92f, 1.08f);
        if (wetFootstep) volume *= kWetFootstepVolumeScale;
        volume *= kFootstepVolumeScale;
        audio_.PlayRandom(sound, AudioBus::Effects, pos, volume, false);
        EmitPlayerAudibleSound(pos, FootstepHearingRadius(walkT, runT, false, wetFootstep), 0.86f);
    }

    void PlaySparkSoundAt(XMFLOAT3 pos, float intensity = 1.0f) {
        audio_.PlayRandom(GameSound::ElectricCrackle, AudioBus::Effects, pos,
            std::clamp(0.30f + intensity * 0.18f, 0.25f, 1.0f), true, AudioOcclusionFor(pos));
    }

    void PlayNeonFlickerStarterClickAt(XMFLOAT3 pos) {
        audio_.PlayRandom(GameSound::NeonFlickerStarterClick, AudioBus::Effects, pos, 0.35f, true, AudioOcclusionFor(pos));
    }

    void PlayLightBulbBreakSoundAt(XMFLOAT3 pos, float intensity = 1.0f, bool emitPlayerNoise = true) {
        audio_.PlayRandom(GameSound::LightBulbBreak, AudioBus::Effects, pos,
            std::clamp(1.55f + intensity * 0.55f, 1.10f, 2.75f),
            runtimeMode_ != RendererRuntimeMode::MainMenu,
            runtimeMode_ != RendererRuntimeMode::MainMenu ? std::min(AudioOcclusionFor(pos), 1.15f) : 0.0f);
        if (emitPlayerNoise) {
            EmitPlayerAudibleSound(pos, LightBulbBreakHearingRadius(), 1.35f);
        }
    }

    void PlayAirVentDustPuffAt(XMFLOAT3 pos, float intensity = 1.0f) {
        audio_.PlayRandom(GameSound::AirVentDustPuff, AudioBus::Effects, pos,
            std::clamp(0.48f + intensity * 0.20f, 0.42f, 0.86f), true, AudioOcclusionFor(pos));
    }

    void UpdateMenuDoorAudio() {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) {
            menuDoorAudioPrimed_ = false;
            menuDoorAudioOpen_ = false;
            menuDoorCloseCreakPlayed_ = false;
            menuDoorCloseLockPlayed_ = false;
            previousMenuDoorAudioOpen_ = 0.0f;
            menuDoorAudioPeakOpen_ = 0.0f;
            return;
        }

        if (!menuDoorAudioPrimed_) {
            menuDoorAudioPrimed_ = true;
            menuDoorAudioOpen_ = menuDoorOpen_ >= 0.035f;
            previousMenuDoorAudioOpen_ = menuDoorOpen_;
            menuDoorAudioPeakOpen_ = menuDoorOpen_;
            menuDoorCloseCreakPlayed_ = false;
            menuDoorCloseLockPlayed_ = menuDoorOpen_ < 0.10f;
            return;
        }

        constexpr float kStartThreshold = 0.035f;
        constexpr float kCloseCreakMinOpen = 0.25f;
        constexpr float kLockMinOpen = 0.035f;
        constexpr float kLockThreshold = 0.10f;
        bool opening = menuDoorOpen_ > previousMenuDoorAudioOpen_ + 0.0015f;
        bool closing = previousMenuDoorAudioOpen_ > menuDoorOpen_ + 0.0015f;
        bool crossedLockThreshold = previousMenuDoorAudioOpen_ > kLockThreshold && menuDoorOpen_ <= kLockThreshold;

        if (opening) {
            menuDoorCloseCreakPlayed_ = false;
            menuDoorCloseLockPlayed_ = false;
            if (!menuDoorAudioOpen_ && menuDoorOpen_ >= kStartThreshold) {
                audio_.PlayRandom(GameSound::DoorOpenCreak, AudioBus::Effects, exitDoorCenter_, 0.72f, true,
                    AudioOcclusionFor(exitDoorCenter_));
                menuDoorAudioOpen_ = true;
            }
        }

        if (menuDoorOpen_ > menuDoorAudioPeakOpen_) {
            menuDoorAudioPeakOpen_ = menuDoorOpen_;
        }

        if (closing && menuDoorAudioPeakOpen_ >= kCloseCreakMinOpen && !menuDoorCloseCreakPlayed_) {
            audio_.PlayRandom(GameSound::DoorCloseCreak, AudioBus::Effects, exitDoorCenter_, 0.66f, true,
                AudioOcclusionFor(exitDoorCenter_));
            menuDoorCloseCreakPlayed_ = true;
            menuDoorAudioOpen_ = false;
        }

        if (closing && menuDoorAudioPeakOpen_ >= kLockMinOpen && crossedLockThreshold && !menuDoorCloseLockPlayed_) {
            audio_.PlayRandom(GameSound::DoorCloseLock, AudioBus::Effects, exitDoorCenter_, 1.20f, false);
            menuDoorAudioOpen_ = false;
            menuDoorCloseCreakPlayed_ = false;
            menuDoorCloseLockPlayed_ = true;
            menuDoorAudioPeakOpen_ = 0.0f;
        }

        previousMenuDoorAudioOpen_ = menuDoorOpen_;
    }

    void ScheduleDelayedAudio(size_t sampleIndex, GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, float delay,
                              float frequencyRatio = 1.0f, bool spatial = true,
                              AudioToneProfile toneProfile = AudioToneProfile::Normal) {
        if (delayedAudioEvents_.size() >= 24) delayedAudioEvents_.erase(delayedAudioEvents_.begin());
        DelayedAudioEvent e{};
        e.sampleIndex = sampleIndex;
        e.sound = sound;
        e.bus = bus;
        e.pos = pos;
        e.volume = volume;
        e.delay = std::max(0.0f, delay);
        e.frequencyRatio = frequencyRatio;
        e.toneProfile = toneProfile;
        e.spatial = spatial;
        delayedAudioEvents_.push_back(e);
    }

    void UpdateDelayedAudio(float dt) {
        for (size_t i = 0; i < delayedAudioEvents_.size();) {
            DelayedAudioEvent& e = delayedAudioEvents_[i];
            e.delay -= dt;
            if (e.delay <= 0.0f) {
                audio_.PlaySample(e.sampleIndex, e.bus, e.pos, e.volume, e.spatial, e.frequencyRatio, e.sound,
                    e.spatial ? AudioOcclusionFor(e.pos) : 0.0f, e.toneProfile);
                delayedAudioEvents_.erase(delayedAudioEvents_.begin() + static_cast<std::ptrdiff_t>(i));
                continue;
            }
            ++i;
        }
    }

    XMFLOAT3 PickVentGroanEmitter() {
        if (steamEmitters_.empty()) {
            XMFLOAT3 away = Normalize3({monster_.x - camera_.x, 0.0f, monster_.z - camera_.z}, {std::sin(yaw_), 0.0f, std::cos(yaw_)});
            return {camera_.x + away.x * maze_.TileAverage() * 2.4f, settings_.wallHeightMeters * 0.82f, camera_.z + away.z * maze_.TileAverage() * 2.4f};
        }

        float tile = std::max(0.1f, maze_.TileAverage());
        float bestScore = -1000000.0f;
        const SteamEmitter* best = nullptr;
        for (const SteamEmitter& emitter : steamEmitters_) {
            float dx = emitter.pos.x - camera_.x;
            float dz = emitter.pos.z - camera_.z;
            float distTiles = std::sqrt(dx * dx + dz * dz) / tile;
            float monsterDx = emitter.pos.x - monster_.x;
            float monsterDz = emitter.pos.z - monster_.z;
            float monsterDistTiles = std::sqrt(monsterDx * monsterDx + monsterDz * monsterDz) / tile;
            float farEnough = SmoothStep(1.15f, 2.45f, distTiles);
            float notTooFar = 1.0f - SmoothStep(7.0f, 10.0f, distTiles);
            float monsterBias = 1.0f - SmoothStep(0.0f, 7.0f, monsterDistTiles);
            float blockedBias = AudioRayClear({camera_.x, 1.15f, camera_.z}, emitter.pos) ? 0.0f : 0.35f;
            float score = farEnough * notTooFar * (0.70f + monsterBias * 0.55f + blockedBias) + RandRange(0.0f, 0.32f);
            if (score > bestScore) {
                bestScore = score;
                best = &emitter;
            }
        }
        return best ? best->pos : steamEmitters_[static_cast<size_t>(rng_() % steamEmitters_.size())].pos;
    }

    void PlayVentMonsterGroan() {
        size_t sampleIndex = audio_.PickRandomSample(GameSound::MonsterGrowl);
        if (sampleIndex == static_cast<size_t>(-1)) return;

        XMFLOAT3 pos = PickVentGroanEmitter();
        float tile = std::max(0.1f, maze_.TileAverage());
        float monsterTiles = MonsterDistance() / tile;
        float monsterDistanceGain = Lerp(0.16f, 1.0f, 1.0f - SmoothStep(2.5f, 6.6f, monsterTiles));
        float baseVolume = RandRange(0.010f, 0.020f) * monsterDistanceGain;
        float pitch = RandRange(1.08f, 1.22f);
        float ductOcclusion = std::min(8.0f, AudioOcclusionFor(pos) + 1.65f + SmoothStep(3.0f, 6.6f, monsterTiles) * 1.25f);
        audio_.PlaySample(sampleIndex, AudioBus::Monster, pos, baseVolume, true, pitch, GameSound::MonsterGrowl,
            ductOcclusion, AudioToneProfile::MetallicVent);

        XMFLOAT3 ductDir = Normalize3({monster_.x - camera_.x, 0.0f, monster_.z - camera_.z}, {std::sin(yaw_), 0.0f, std::cos(yaw_)});
        XMFLOAT3 echoA{pos.x + ductDir.z * tile * RandRange(-0.52f, 0.52f), pos.y, pos.z - ductDir.x * tile * RandRange(-0.52f, 0.52f)};
        XMFLOAT3 echoB{pos.x + ductDir.x * tile * RandRange(0.45f, 1.35f), pos.y, pos.z + ductDir.z * tile * RandRange(0.45f, 1.35f)};
        ScheduleDelayedAudio(sampleIndex, GameSound::MonsterGrowl, AudioBus::Monster, echoA, baseVolume * 0.28f,
            RandRange(0.085f, 0.15f), pitch * RandRange(1.035f, 1.075f), true, AudioToneProfile::MetallicVent);
        ScheduleDelayedAudio(sampleIndex, GameSound::MonsterGrowl, AudioBus::Monster, echoB, baseVolume * 0.14f,
            RandRange(0.19f, 0.34f), pitch * RandRange(0.93f, 0.98f), true, AudioToneProfile::MetallicVent);
    }

    bool MonsterAlertAudioActive() const {
        return monsterChasingVisible_ || monsterRecognizedForChase_ || monsterHasLastKnown_ ||
            monsterHasSound_ || chaseMemoryTimer_ > 0.0f || chasePanic_ > 0.08f;
    }

    void PlayLayeredMonsterSound(GameSound sound, XMFLOAT3 pos, float volume, float pitchMin = 0.86f, float pitchMax = 1.10f) {
        size_t first = audio_.PickRandomSample(sound);
        if (first == static_cast<size_t>(-1)) return;
        size_t second = audio_.PickRandomSampleExcept(sound, first);
        float firstPitch = RandRange(pitchMin, pitchMax);
        float initialOcclusion = AudioOcclusionFor(pos);
        audio_.PlaySample(first, AudioBus::Monster, pos, volume * RandRange(0.84f, 1.08f), true, firstPitch, sound,
            initialOcclusion);
        if (second != static_cast<size_t>(-1)) {
            audio_.PlaySample(second, AudioBus::Monster, pos, volume * RandRange(0.52f, 0.78f), true,
                std::clamp(firstPitch * RandRange(0.82f, 1.18f), 0.50f, 2.0f), sound, initialOcclusion);
        }
    }

    void ScheduleLayeredMonsterSound(GameSound sound, XMFLOAT3 pos, float volume, float delay, float pitchMin = 0.86f, float pitchMax = 1.10f) {
        size_t first = audio_.PickRandomSample(sound);
        if (first == static_cast<size_t>(-1)) return;
        size_t second = audio_.PickRandomSampleExcept(sound, first);
        float firstPitch = RandRange(pitchMin, pitchMax);
        ScheduleDelayedAudio(first, sound, AudioBus::Monster, pos, volume * RandRange(0.84f, 1.08f), delay, firstPitch);
        if (second != static_cast<size_t>(-1)) {
            ScheduleDelayedAudio(second, sound, AudioBus::Monster, pos, volume * RandRange(0.52f, 0.78f),
                delay + RandRange(0.015f, 0.055f), std::clamp(firstPitch * RandRange(0.82f, 1.18f), 0.50f, 2.0f));
        }
    }

    void PlayMonsterAlertGroan(float volume = 0.82f) {
        PlayLayeredMonsterSound(GameSound::MonsterGrowl, MonsterSoundOrigin(),
            std::clamp(volume * RandRange(0.86f, 1.12f), 0.18f, 1.25f), 0.82f, 1.12f);
    }

    float MonsterAlertVocalVolume(bool visibleChase, float closePressure) const {
        if (visibleChase) return Lerp(0.92f, 1.18f, closePressure);
        if (monsterHasLastKnown_ || monsterHasSound_ || chaseMemoryTimer_ > 0.0f) return Lerp(0.52f, 0.78f, closePressure);
        return Lerp(0.28f, 0.44f, closePressure);
    }

    void UpdateMonsterVocalAudio(float dt) {
        monsterSpottedScreamCooldown_ = std::max(0.0f, monsterSpottedScreamCooldown_ - dt);
        if (monsterPreview_ || !IsPlayableSimulationMode(runtimeMode_)) {
            monsterAlertAudioActive_ = false;
            monsterAlertVocalTimer_ = 0.0f;
            return;
        }

        bool alert = MonsterAlertAudioActive();
        if (!alert) {
            monsterAlertAudioActive_ = false;
            monsterAlertVocalTimer_ = 0.0f;
            nextMonsterGrowlSeconds_ -= dt;
            if (nextMonsterGrowlSeconds_ <= 0.0f) {
                PlayMonsterAlertGroan(RandRange(0.20f, 0.32f));
                nextMonsterGrowlSeconds_ = RandRange(12.0f, 36.0f);
            }
            return;
        }

        float distance = MonsterDistance();
        float closePressure = 1.0f - SmoothStep(2.0f, 10.0f, distance);
        bool visibleChase = monsterChasingVisible_ || monsterRecognizedForChase_;
        float alertVolume = MonsterAlertVocalVolume(visibleChase, closePressure);

        if (!monsterAlertAudioActive_ && visibleChase && monsterSpottedScreamCooldown_ <= 0.0f) {
            PlayLayeredMonsterSound(GameSound::MonsterSpottedScream, MonsterSoundOrigin(), alertVolume, 0.88f, 1.08f);
            ScheduleLayeredMonsterSound(GameSound::MonsterGrowl, MonsterSoundOrigin(),
                alertVolume * RandRange(0.74f, 0.92f), RandRange(0.55f, 0.95f), 0.82f, 1.10f);
            monsterSpottedScreamCooldown_ = RandRange(5.2f, 7.4f);
            monsterAlertVocalTimer_ = RandRange(1.0f, 1.8f);
        }

        monsterAlertAudioActive_ = true;
        monsterAlertVocalTimer_ = std::max(0.0f, monsterAlertVocalTimer_ - dt);
        if (monsterAlertVocalTimer_ > 0.0f) return;

        if (visibleChase && monsterSpottedScreamCooldown_ <= 0.0f && RandRange(0.0f, 1.0f) < 0.34f) {
            PlayLayeredMonsterSound(GameSound::MonsterSpottedScream, MonsterSoundOrigin(), alertVolume * RandRange(0.88f, 1.06f), 0.88f, 1.08f);
            monsterSpottedScreamCooldown_ = RandRange(5.2f, 8.0f);
            monsterAlertVocalTimer_ = RandRange(1.2f, 2.1f);
            return;
        }

        PlayMonsterAlertGroan(alertVolume);
        monsterAlertVocalTimer_ = visibleChase
            ? RandRange(1.05f, 2.35f)
            : RandRange(4.0f, 8.5f);
    }

    void UpdateVentMonsterGroans(float dt) {
        if (monsterPreview_ || !IsPlayableSimulationMode(runtimeMode_) || deathActive_ || exitTransitionActive_) {
            ventMonsterGroanTimer_ = std::min(ventMonsterGroanTimer_, 2.0f);
            ventMonsterGroanCooldown_ = std::max(0.0f, ventMonsterGroanCooldown_ - dt);
            return;
        }

        float tile = std::max(0.1f, maze_.TileAverage());
        float monsterTiles = MonsterDistance() / tile;
        float aroundFourTiles = 1.0f - Clamp01(std::abs(monsterTiles - 4.0f) / 2.15f);
        bool eligible = aroundFourTiles > 0.0f && !IsThreatVisible() && !ChasePanicActive();
        if (!eligible) {
            ventMonsterGroanTimer_ = std::min(ventMonsterGroanTimer_, RandRange(2.2f, 5.4f));
            ventMonsterGroanCooldown_ = std::max(0.0f, ventMonsterGroanCooldown_ - dt);
            return;
        }

        ventMonsterGroanTimer_ = std::max(0.0f, ventMonsterGroanTimer_ - dt);
        ventMonsterGroanCooldown_ = std::max(0.0f, ventMonsterGroanCooldown_ - dt);
        if (ventMonsterGroanTimer_ > 0.0f || ventMonsterGroanCooldown_ > 0.0f) return;

        float chance = Lerp(0.18f, 0.52f, aroundFourTiles);
        if (RandRange(0.0f, 1.0f) < chance) {
            PlayVentMonsterGroan();
            ventMonsterGroanCooldown_ = RandRange(18.0f, 42.0f);
        }
        ventMonsterGroanTimer_ = RandRange(7.0f, 18.0f);
    }

    bool AudioRayClear(XMFLOAT3 from, XMFLOAT3 to) const {
        return AudioWallBlocksBetween(from, to) == 0;
    }

    int AudioWallBlocksBetween(XMFLOAT3 from, XMFLOAT3 to) const {
        Tile fromTile = maze_.TileFromWorld(from.x, from.z);
        Tile toTile = maze_.TileFromWorld(to.x, to.z);
        if (!maze_.InBounds(fromTile.x, fromTile.y) || !maze_.InBounds(toTile.x, toTile.y)) return 4;
        if (fromTile == toTile) return maze_.IsOpen(fromTile.x, fromTile.y) ? 0 : 1;
        float dx = to.x - from.x;
        float dz = to.z - from.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        int steps = std::clamp(static_cast<int>(dist / std::max(0.05f, maze_.TileMinimum() * 0.12f)), 6, 160);
        int blocks = 0;
        Tile previous{-100000, -100000};
        for (int i = 1; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            Tile sample = maze_.TileFromWorld(from.x + dx * t, from.z + dz * t);
            if (!maze_.InBounds(sample.x, sample.y)) {
                ++blocks;
                if (blocks >= 8) return blocks;
                continue;
            }
            if (!maze_.IsOpen(sample.x, sample.y) && !(sample == previous)) {
                ++blocks;
                if (blocks >= 8) return blocks;
            }
            previous = sample;
        }
        return blocks;
    }

    float AudioOcclusionFor(XMFLOAT3 source) const {
        if (monsterPreview_ || runtimeMode_ == RendererRuntimeMode::MainMenu) return 0.0f;
        float dist = DistanceToPoint(source);
        if (dist > std::max(72.0f, maze_.TileAverage() * 32.0f)) return 0.0f;
        XMFLOAT3 listener{camera_.x, 1.30f, camera_.z};
        source.y = std::clamp(source.y, 0.15f, settings_.wallHeightMeters - 0.08f);
        Tile listenerTile = maze_.TileFromWorld(listener.x, listener.z);
        Tile sourceTile = maze_.TileFromWorld(source.x, source.z);
        if (listenerTile == sourceTile) return 0.0f;

        XMFLOAT3 forward = Normalize3(Sub3(source, listener), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 right = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, forward), {1.0f, 0.0f, 0.0f});
        float radius = std::clamp(maze_.TileMinimum() * 0.16f, 0.12f, 0.30f);
        std::array<float, 5> offsets{{0.0f, -1.0f, 1.0f, -0.45f, 0.45f}};
        int minBlocks = 8;
        for (float offset : offsets) {
            XMFLOAT3 l = Add3(listener, Scale3(right, offset * radius));
            XMFLOAT3 s = Add3(source, Scale3(right, -offset * radius * 0.60f));
            minBlocks = std::min(minBlocks, AudioWallBlocksBetween(l, s));
        }
        return static_cast<float>(minBlocks);
    }

    void SetupPersistentAudioEmitters() {
        audio_.StopAll();
        nextMonsterGrowlSeconds_ = RandRange(12.0f, 36.0f);
        monsterSpottedScreamCooldown_ = 0.0f;
        ventMonsterGroanTimer_ = RandRange(7.0f, 18.0f);
        ventMonsterGroanCooldown_ = RandRange(14.0f, 32.0f);
        previousStepAudioPhase_ = stepPhase_;
        exitDoorOpenSoundPlayed_ = false;
        exitDoorCloseCreakSoundPlayed_ = false;
        exitDoorCloseSoundPlayed_ = false;
        menuDoorAudioPrimed_ = false;
        menuDoorAudioOpen_ = false;
        menuDoorCloseCreakPlayed_ = false;
        menuDoorCloseLockPlayed_ = false;
        previousMenuDoorAudioOpen_ = 0.0f;
        menuDoorAudioPeakOpen_ = 0.0f;
        delayedAudioEvents_.clear();
        lampHumRefreshTimer_ = 0.0f;
        monsterAlertVocalTimer_ = 0.0f;
        monsterAlertAudioActive_ = false;
        if (!audioReady_) return;
        UpdatePersistentLampHumVoices(0.0f, true);
    }

    void UpdateAudio(float dt) {
        if (!audioReady_) return;
        audio_.ApplySettings(settings_);
        audio_.SetListener(camera_, DirectionFromYawPitch(yaw_, lookPitch_));
        UpdatePlayerAudibleSoundPulses(dt);
        UpdateMenuDoorAudio();
        UpdatePersistentLampHumVoices(dt);
        UpdateLampFlickerStarterClicks(dt);
        UpdateDelayedAudio(dt);
        UpdateWetDripAudio(dt);
        UpdateVentMonsterGroans(dt);

        UpdateMonsterVocalAudio(dt);

        if (IsPlayableSimulationMode(runtimeMode_) && smoothedMoveSpeed_ > 0.05f) {
            float oldPhase = previousStepAudioPhase_;
            float newPhase = stepPhase_;
            if (newPhase < oldPhase) newPhase += kPi * 2.0f;
            int oldStep = FootstepDownBobIndex(oldPhase);
            int newStep = FootstepDownBobIndex(newPhase);
            if (newStep > oldStep) PlayFootstepSound();
            previousStepAudioPhase_ = stepPhase_;
        } else {
            previousStepAudioPhase_ = stepPhase_;
        }

        if (exitTransitionActive_) {
            if (!exitDoorOpenSoundPlayed_ && exitDoorAngle_ > 0.02f) {
                audio_.PlayRandom(GameSound::DoorOpenCreak, AudioBus::Effects, exitDoorCenter_, 0.90f, true,
                    AudioOcclusionFor(exitDoorCenter_));
                exitDoorOpenSoundPlayed_ = true;
            }
            if (!exitDoorCloseCreakSoundPlayed_ && exitTransitionTimer_ > settings_.exitDoorOpenSeconds + settings_.exitStepSeconds * 0.15f) {
                audio_.PlayRandom(GameSound::DoorCloseCreak, AudioBus::Effects, exitDoorCenter_, 0.76f, true,
                    AudioOcclusionFor(exitDoorCenter_));
                exitDoorCloseCreakSoundPlayed_ = true;
            }
            if (!exitDoorCloseSoundPlayed_ && exitTransitionTimer_ > settings_.exitDoorOpenSeconds + settings_.exitStepSeconds * 0.55f) {
                audio_.PlayRandom(GameSound::DoorCloseLock, AudioBus::Effects, exitDoorCenter_, 0.78f, true,
                    AudioOcclusionFor(exitDoorCenter_));
                exitDoorCloseSoundPlayed_ = true;
            }
        }

        audio_.Update(dt, [&](XMFLOAT3 pos) {
            return AudioOcclusionFor(pos);
        });
    }
