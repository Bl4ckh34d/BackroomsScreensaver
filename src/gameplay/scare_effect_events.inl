    bool PlayerLooksAt(const XMFLOAT3& p, float maxDist, float minDot) const {
        float dx = p.x - camera_.x;
        float dy = p.y - camera_.y;
        float dz = p.z - camera_.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < 0.001f || dist > maxDist) return false;
        XMFLOAT3 viewDir = DirectionFromYawPitch(yaw_, lookPitch_);
        float dot = (dx * viewDir.x + dy * viewDir.y + dz * viewDir.z) / dist;
        if (dot < minDot) return false;
        Tile targetTile = maze_.TileFromWorld(p.x, p.z);
        return maze_.LineClear(CameraTile(), targetTile);
    }

    float DistanceToPoint(const XMFLOAT3& p) const {
        float dx = p.x - camera_.x;
        float dy = p.y - camera_.y;
        float dz = p.z - camera_.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    float ScareSensoryWeight(const XMFLOAT3& p, float visualDistance, float minDot, float loudDistance) const {
        if (PlayerLooksAt(p, visualDistance, minDot)) return 1.0f;
        float dist = DistanceToPoint(p);
        if (dist > loudDistance) return 0.0f;
        float close = Clamp01((loudDistance - dist) / std::max(0.1f, loudDistance));
        return 0.42f + close * 0.46f;
    }

    bool ScareSourceAhead(const XMFLOAT3& p, float minDistance, float maxDistance, int maxPathTiles, float minForwardDot) const {
        float dx = p.x - camera_.x;
        float dz = p.z - camera_.z;
        float horizontalDist = std::sqrt(dx * dx + dz * dz);
        if (horizontalDist < minDistance || horizontalDist > maxDistance) return false;

        Tile cameraTile = CameraTile();
        Tile sourceTile = maze_.TileFromWorld(p.x, p.z);
        if (!maze_.LineClear(cameraTile, sourceTile)) return false;

        XMFLOAT3 forward = Forward();
        float facing = horizontalDist > 0.001f ? (dx * forward.x + dz * forward.z) / horizontalDist : 1.0f;
        bool upcomingPath = false;
        size_t first = std::min(pathIndex_, path_.size());
        size_t last = std::min(path_.size(), first + static_cast<size_t>(std::max(1, maxPathTiles)));
        for (size_t i = first; i < last; ++i) {
            if (path_[i] == sourceTile) {
                upcomingPath = true;
                break;
            }
        }
        if (sourceTile == cameraTile && facing > 0.18f) {
            upcomingPath = true;
        }
        return (upcomingPath && facing > -0.18f) || facing >= minForwardDot;
    }

    void EmitSparkBurstAt(const XMFLOAT3& pos, float intensity = 1.0f) {
        if (!settings_.sparkParticles || settings_.sparkMaxParticles <= 0) return;
        int count = static_cast<int>((5.0f + RandRange(0.0f, 8.9f)) * intensity);
        for (int i = 0; i < count && sparks_.size() < static_cast<size_t>(settings_.sparkMaxParticles); ++i) {
            float yaw = RandRange(-1.15f, 1.15f) + LampHash(pos.x, pos.z) * kPi;
            float speed = RandRange(0.35f, 1.25f) * (0.85f + intensity * 0.30f);
            SparkParticle sp{};
            sp.pos = pos;
            sp.vel = {std::sin(yaw) * speed, RandRange(0.15f, 0.92f) * (0.7f + intensity * 0.45f), std::cos(yaw) * speed};
            sp.age = 0.0f;
            sp.life = RandRange(0.55f, 1.35f) * (0.85f + intensity * 0.12f);
            sp.size = RandRange(0.018f, 0.040f) * settings_.sparkSize * (0.9f + intensity * 0.12f);
            sparks_.push_back(sp);
        }
        SparkFlash flash{};
        flash.pos = pos;
        flash.age = 0.0f;
        flash.life = std::clamp(0.16f + intensity * 0.035f, 0.16f, 0.42f);
        flash.intensity = std::clamp(intensity * 2.2f, 0.3f, 12.0f);
        sparkFlashes_.push_back(flash);
    }

    void SpawnSparkBurst(const SparkEmitter& emitter, float intensity = 1.0f) {
        EmitSparkBurstAt(emitter.pos, intensity);
    }

    void ScheduleSparkChain(const XMFLOAT3& pos, float intensity, int bursts) {
        SparkChain chain{};
        chain.pos = pos;
        chain.timer = RandRange(0.055f, 0.16f);
        chain.intensity = intensity;
        chain.remaining = std::max(0, bursts);
        sparkChains_.push_back(chain);
    }

    void MarkLampDamagePixel(Tile tile, float damage) {
        if (!maze_.InBounds(tile.x, tile.y) || lampDamagePixels_.empty()) return;
        size_t index = static_cast<size_t>(tile.y * maze_.w + tile.x);
        if (index >= lampDamagePixels_.size()) return;
        uint8_t value = Byte(damage);
        if (value > lampDamagePixels_[index]) {
            lampDamagePixels_[index] = value;
            lampDamageDirty_ = true;
        }
    }

    void BreakRuntimeLamp(RuntimeLampState& lamp) {
        if (lamp.broken) return;
        lamp.broken = true;
        lamp.damage = 1.0f;
        MarkLampDamagePixel(lamp.tile, lamp.damage);
        if (settings_.sparkParticles) {
            float intensity = std::max(2.2f, PickBrokenLampSparkIntensity() * 1.18f);
            EmitSparkBurstAt(lamp.pos, intensity);
            ScheduleSparkChain(lamp.pos, intensity * settings_.effectBrokenLampChainIntensityScale,
                std::max(2, PickBrokenLampChainBursts() + 1));
        }
    }

    void UpdateMonsterLampDamage(float dt) {
        if (dt <= 0.0f || monsterPreview_ || gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView) return;
        if (runtimeLamps_.empty() || lampDamagePixels_.empty()) return;

        Tile monsterTile = MonsterTile();
        if (!maze_.IsOpen(monsterTile.x, monsterTile.y)) return;
        float tileAvg = std::max(0.1f, maze_.TileAverage());
        float influenceRadius = std::max(tileAvg * 2.15f, 3.25f);
        float breakRadius = influenceRadius * 0.72f;

        for (RuntimeLampState& lamp : runtimeLamps_) {
            if (lamp.broken) continue;

            float dx = lamp.pos.x - monster_.x;
            float dz = lamp.pos.z - monster_.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            bool affected = dist <= influenceRadius && maze_.LineClear(monsterTile, lamp.tile);
            float oldDamage = lamp.damage;

            if (affected) {
                float proximity = Clamp01((influenceRadius - dist) / std::max(0.001f, influenceRadius));
                float close = SmoothStep(0.0f, 1.0f, proximity);
                lamp.damage = std::max(lamp.damage, 0.12f * close);
                lamp.damage += dt * (0.10f + close * 0.92f + close * close * 0.65f);
                if (dist < breakRadius && lamp.damage > 0.58f) {
                    lamp.damage += dt * Lerp(0.18f, 0.58f, close);
                }
            } else if (lamp.damage > 0.055f) {
                lamp.damage += dt * (0.014f + lamp.damage * 0.036f);
            }

            lamp.damage = Clamp01(lamp.damage);
            if (lamp.damage > oldDamage + 0.001f) {
                MarkLampDamagePixel(lamp.tile, lamp.damage);
            }

            float fail = SmoothStep(0.28f, 0.96f, lamp.damage);
            if (fail > 0.001f) {
                lamp.sparkTimer -= dt * Lerp(0.85f, affected ? 3.5f : 1.7f, fail);
                if (lamp.sparkTimer <= 0.0f) {
                    if (settings_.sparkParticles) {
                        float intensity = Lerp(0.42f, 2.55f, fail);
                        EmitSparkBurstAt(lamp.pos, intensity);
                        if (fail > 0.68f && RandRange(0.0f, 1.0f) < Lerp(0.16f, 0.62f, fail)) {
                            ScheduleSparkChain(lamp.pos, intensity * settings_.effectBrokenLampChainIntensityScale,
                                std::max(1, PickBrokenLampChainBursts() / 2));
                        }
                    }
                    float cooldown = Lerp(1.75f, 0.12f, fail);
                    lamp.sparkTimer = RandRange(cooldown * 0.55f, cooldown * 1.25f);
                    if (affected && fail > 0.80f) {
                        lamp.damage = Clamp01(lamp.damage + 0.025f);
                        MarkLampDamagePixel(lamp.tile, lamp.damage);
                    }
                }
            }

            if (lamp.damage >= 0.995f) {
                BreakRuntimeLamp(lamp);
            }
        }
    }

    void SpawnSteamBurst(const SteamEmitter& emitter, float intensity = 1.0f) {
        int count = static_cast<int>(RandRange(12.0f, 26.0f) * intensity);
        for (int i = 0; i < count && steam_.size() < 420; ++i) {
            float side = RandRange(-0.42f, 0.42f);
            float rise = RandRange(-0.08f, 0.18f);
            XMFLOAT3 right{emitter.dir.z, 0.0f, -emitter.dir.x};
            SteamParticle sp{};
            sp.pos = Add3(emitter.pos, Add3(Scale3(right, side), {0.0f, rise, 0.0f}));
            float spread = RandRange(-0.32f, 0.32f);
            sp.vel = {
                emitter.dir.x * RandRange(0.55f, 1.25f) * intensity + right.x * spread,
                RandRange(0.16f, 0.62f) * intensity,
                emitter.dir.z * RandRange(0.55f, 1.25f) * intensity + right.z * spread
            };
            sp.age = 0.0f;
            sp.life = RandRange(1.0f, 2.4f) * (0.85f + intensity * 0.18f);
            sp.size = RandRange(0.22f, 0.52f) * (0.85f + intensity * 0.25f);
            steam_.push_back(sp);
        }
    }

    bool SpawnVentDrop(const SteamEmitter& emitter) {
        if (ventDrops_.size() >= kMaxVentDrops) return false;
        VentDrop d{};
        d.pos = emitter.pos;
        d.vel = {emitter.dir.x * RandRange(0.45f, 0.95f), RandRange(0.22f, 0.75f), emitter.dir.z * RandRange(0.45f, 0.95f)};
        d.yaw = std::atan2(emitter.dir.x, emitter.dir.z) + RandRange(-0.4f, 0.4f);
        d.roll = RandRange(-0.35f, 0.35f);
        d.angular = RandRange(-4.8f, 4.8f);
        d.life = RandRange(2.5f, 4.6f);
        ventDrops_.push_back(d);
        return true;
    }

    void BeginVentReaction(const SteamEmitter& emitter, float sensory) {
        if (IsThreatVisible() || ChasePanicActive()) return;
        ventReactionTarget_ = emitter.pos;
        ventReactionAway_ = Normalize3({camera_.x - emitter.pos.x, 0.0f, camera_.z - emitter.pos.z}, emitter.dir);
        ventReactionDuration_ = RandRange(1.35f, 1.85f) * Lerp(0.90f, 1.12f, Clamp01(sensory));
        ventReactionLookDelay_ = RandRange(0.14f, 0.38f) * Lerp(1.08f, 0.82f, Clamp01(sensory));
        ventReactionBackDuration_ = RandRange(0.58f, 0.96f);
        ventReactionScanSeed_ = RandRange(0.0f, kPi * 2.0f);
        ventReactionTimer_ = ventReactionDuration_;
        stopTimer_ = 0.0f;
        headScanTimer_ = 0.0f;
        junctionScanActive_ = false;
        lookBack_ = false;
        propLookTimer_ = 0.0f;
        bloodFocusTimer_ = 0.0f;
        flashlightAgitation_ = std::max(flashlightAgitation_, 0.55f + sensory * 0.30f);
    }

    void UpdateScareEvents(float dt) {
        scareCooldown_ = std::max(0.0f, scareCooldown_ - dt);
        fleshFlickerTimer_ = std::max(0.0f, fleshFlickerTimer_ - dt);
        bloodWorldFlickerTimer_ = std::max(0.0f, bloodWorldFlickerTimer_ - dt);
        float scareFrequency = JumpscareFrequency();
        float scareScale = ScareCooldownScale();
        if (scareFrequency <= 0.001f) {
            fleshFlickerTimer_ = 0.0f;
            fleshFlickerCooldown_ = 1000000.0f;
            bloodWorldFlickerTimer_ = 0.0f;
            bloodWorldFlickerCooldown_ = 1000000.0f;
            return;
        }
        if (IsThreatVisible() || ChasePanicActive()) {
            scareCooldown_ = std::max(scareCooldown_, 0.80f);
            ventReactionTimer_ = 0.0f;
            return;
        }
        if (settings_.fleshFlicker) {
            fleshFlickerCooldown_ = std::max(0.0f, fleshFlickerCooldown_ - dt);
            if (fleshFlickerCooldown_ <= 0.0f && fleshFlickerTimer_ <= 0.0f && scareCooldown_ <= 0.0f) {
                fleshFlickerDuration_ = RandRange(settings_.fleshFlickerDuration * 0.82f, settings_.fleshFlickerDuration * 1.24f);
                fleshFlickerTimer_ = fleshFlickerDuration_;
                fleshFlickerCooldown_ = RandRange(settings_.fleshFlickerMinSeconds, settings_.fleshFlickerMaxSeconds) * scareScale;
                scareCooldown_ = std::max(scareCooldown_, fleshFlickerDuration_ + RandRange(8.0f, 18.0f) * scareScale);
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.62f);
                AddDread(settings_.dreadFleshGain);
            }
        } else {
            fleshFlickerTimer_ = 0.0f;
            fleshFlickerCooldown_ = 1000000.0f;
        }
        if (settings_.bloodWorldFlicker && settings_.bloodWorldCoverage > 0.001f) {
            bloodWorldFlickerCooldown_ = std::max(0.0f, bloodWorldFlickerCooldown_ - dt);
            if (bloodWorldActivationTime_ < -900.0f &&
                bloodWorldFlickerCooldown_ <= 0.0f && bloodWorldFlickerTimer_ <= 0.0f && scareCooldown_ <= 0.0f) {
                bloodWorldFlickerDuration_ = RandRange(settings_.bloodWorldFlickerDuration * 0.82f, settings_.bloodWorldFlickerDuration * 1.24f);
                bloodWorldFlickerTimer_ = bloodWorldFlickerDuration_;
                bloodWorldActivationTime_ = time_;
                bloodWorldFlickerCooldown_ = RandRange(settings_.bloodWorldFlickerMinSeconds, settings_.bloodWorldFlickerMaxSeconds) * scareScale;
                scareCooldown_ = std::max(scareCooldown_, bloodWorldFlickerDuration_ + RandRange(9.0f, 20.0f) * scareScale);
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.72f);
                AddDread(std::max(settings_.dreadJumpscareGain * 0.90f, 0.30f));
            }
        } else {
            bloodWorldFlickerTimer_ = 0.0f;
            bloodWorldFlickerCooldown_ = 1000000.0f;
        }
        if (deathActive_ || exitTransitionActive_) return;

        Tile currentTile = CameraTile();
        bool enteredTile = !(currentTile == scareEventTile_);
        if (enteredTile) {
            scareEventTile_ = currentTile;
        }

        for (SparkEmitter& emitter : sparkEmitters_) {
            if (emitter.triggered) continue;
            if (!ScareSourceAhead(emitter.pos,
                maze_.TileMinimum() * 0.78f,
                maze_.TileAverage() * 2.65f,
                4,
                0.10f)) continue;
            float sensory = std::max(ScareSensoryWeight(emitter.pos, 8.5f, 0.80f, 2.35f), 0.72f);
            if (scareCooldown_ <= 0.0f && sensory > 0.0f) {
                emitter.triggered = true;
                float intensity = PickBrokenLampSparkIntensity();
                AlertMonsterToPlayerTrigger(emitter.pos);
                SpawnSparkBurst(emitter, intensity);
                ScheduleSparkChain(emitter.pos, intensity * settings_.effectBrokenLampChainIntensityScale, PickBrokenLampChainBursts());
                scareCooldown_ = RandRange(9.0f, 18.0f) * scareScale;
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.42f + sensory * 0.43f);
                AddDread(settings_.dreadJumpscareGain *
                    Clamp01(intensity / std::max(0.1f, settings_.effectBrokenLampSparkIntensityMax)) * sensory);
                if (sensory > 0.55f) {
                    stumbleTimer_ = std::max(stumbleTimer_, 0.12f + sensory * 0.06f);
                    stumbleDuration_ = std::max(stumbleDuration_, 0.16f + sensory * 0.06f);
                    stumbleYawOffset_ = RandRange(-0.18f, 0.18f) * sensory;
                }
            }
        }

        for (SteamEmitter& emitter : steamEmitters_) {
            if (emitter.triggered) continue;
            if (!ScareSourceAhead(emitter.pos,
                maze_.TileMinimum() * 0.82f,
                maze_.TileAverage() * 2.80f,
                4,
                0.08f)) continue;
            float sensory = std::max(ScareSensoryWeight(emitter.pos, 7.8f, 0.76f, 2.10f), 0.70f);
            if (sensory > 0.0f && scareCooldown_ <= 0.0f) {
                emitter.triggered = true;
                AlertMonsterToPlayerTrigger(emitter.pos);
                SpawnSteamBurst(emitter, PickAirVentSteamIntensity());
                if (!emitter.panelDropped &&
                    RandRange(0.0f, 1.0f) < settings_.effectAirVentPanelDropChance &&
                    SpawnVentDrop(emitter)) {
                    emitter.panelDropped = true;
                    flashlightAgitation_ = std::max(flashlightAgitation_, 0.34f + sensory * 0.41f);
                    AddDread(settings_.dreadJumpscareGain * 0.78f * sensory);
                }
                scareCooldown_ = RandRange(10.0f, 21.0f) * scareScale;
                AddDread(settings_.dreadJumpscareGain * 0.62f * sensory);
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.28f + sensory * 0.36f);
                BeginVentReaction(emitter, sensory);
            }
        }
    }

    void UpdateSparks(float dt) {
        if (!settings_.sparkParticles) {
            sparks_.clear();
            sparkFlashes_.clear();
            sparkChains_.clear();
            return;
        }
        for (SparkChain& chain : sparkChains_) {
            chain.timer -= dt;
            while (chain.remaining > 0 && chain.timer <= 0.0f) {
                EmitSparkBurstAt(chain.pos, chain.intensity * RandRange(0.68f, 1.18f));
                --chain.remaining;
                chain.timer += RandRange(0.075f, 0.22f);
            }
        }
        sparkChains_.erase(std::remove_if(sparkChains_.begin(), sparkChains_.end(), [](const SparkChain& chain) {
            return chain.remaining <= 0;
        }), sparkChains_.end());

        sparkCooldown_ = 1000000.0f;

        for (SparkParticle& sp : sparks_) {
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
        sparks_.erase(std::remove_if(sparks_.begin(), sparks_.end(), [](const SparkParticle& sp) {
            return sp.age >= sp.life;
        }), sparks_.end());

        for (SparkFlash& flash : sparkFlashes_) {
            flash.age += dt;
        }
        sparkFlashes_.erase(std::remove_if(sparkFlashes_.begin(), sparkFlashes_.end(), [](const SparkFlash& flash) {
            return flash.age >= flash.life;
        }), sparkFlashes_.end());
    }

    void UpdateSteamAndDrops(float dt) {
        for (SteamParticle& sp : steam_) {
            sp.age += dt;
            sp.vel.y += 0.18f * dt;
            sp.vel.x *= std::max(0.0f, 1.0f - dt * 0.42f);
            sp.vel.z *= std::max(0.0f, 1.0f - dt * 0.42f);
            sp.pos.x += sp.vel.x * dt;
            sp.pos.y += sp.vel.y * dt;
            sp.pos.z += sp.vel.z * dt;
        }
        steam_.erase(std::remove_if(steam_.begin(), steam_.end(), [](const SteamParticle& sp) {
            return sp.age >= sp.life;
        }), steam_.end());

        for (VentDrop& d : ventDrops_) {
            d.age += dt;
            if (!d.landed) {
                d.vel.y -= 4.6f * dt;
                d.pos.x += d.vel.x * dt;
                d.pos.y += d.vel.y * dt;
                d.pos.z += d.vel.z * dt;
                d.roll += d.angular * dt;
                if (d.pos.y < kVentDropFloorY || d.age >= d.life) {
                    d.pos.y = kVentDropFloorY;
                    d.vel.y = -d.vel.y * 0.18f;
                    d.vel.x *= 0.38f;
                    d.vel.z *= 0.38f;
                    d.angular *= 0.28f;
                    if (std::abs(d.vel.y) < 0.16f || d.age >= d.life) {
                        d.landed = true;
                        d.vel = {};
                        d.angular = 0.0f;
                        d.roll = kPi * 0.5f;
                        SparkEmitter impact{{d.pos.x, d.pos.y + 0.08f, d.pos.z}, 0.0f};
                        SpawnSparkBurst(impact, 1.6f);
                        AlertMonsterToPlayerTrigger(impact.pos);
                    }
                }
            }
        }
    }
