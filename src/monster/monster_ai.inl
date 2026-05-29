    float MonsterSightDistance() const {
        float visibleDistance = std::max(0.1f, settings_.monsterVisibleDistance);
        if (settings_.fogDarkness > 0.02f) {
            visibleDistance = std::min(visibleDistance, std::max(0.1f, settings_.fogEndMeters));
        }
        return visibleDistance;
    }

    bool MonsterLineOfSightToPlayer() const {
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        if (!maze_.IsOpen(mt.x, mt.y) || !maze_.IsOpen(ct.x, ct.y)) return false;
        float dx = monster_.x - camera_.x;
        float dz = monster_.z - camera_.z;
        float visibleDistance = MonsterSightDistance();
        if (dx * dx + dz * dz > visibleDistance * visibleDistance) return false;
        return maze_.LineClear(mt, ct);
    }

    bool MonsterEyeConeLineOfSightToPlayer() const {
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        if (!maze_.IsOpen(mt.x, mt.y) || !maze_.IsOpen(ct.x, ct.y)) return false;

        XMFLOAT3 eyeOrigin{monster_.x, 1.54f, monster_.z};
        if (monsterEyeWorldCount_ >= 2) {
            eyeOrigin = Scale3(Add3(monsterEyeWorld_[0], monsterEyeWorld_[1]), 0.5f);
        }

        XMFLOAT3 fallbackDir{std::sin(monsterYaw_ + monsterHeadYawOffset_), 0.0f, std::cos(monsterYaw_ + monsterHeadYawOffset_)};
        if (std::abs(monsterHeadPitchOffset_) > 0.0005f) {
            fallbackDir = Normalize3({fallbackDir.x * std::cos(monsterHeadPitchOffset_),
                std::sin(monsterHeadPitchOffset_),
                fallbackDir.z * std::cos(monsterHeadPitchOffset_)}, fallbackDir);
        }
        XMFLOAT3 eyeDir = monsterEyeWorldCount_ >= 2
            ? Normalize3(monsterEyeForward_, fallbackDir)
            : Normalize3(fallbackDir, {0.0f, 0.0f, 1.0f});

        XMFLOAT3 playerFocus{camera_.x, camera_.y + 0.03f, camera_.z};
        XMFLOAT3 toPlayer = Sub3(playerFocus, eyeOrigin);
        float distSq = Dot3(toPlayer, toPlayer);
        float range = MonsterSightDistance();
        if (distSq > range * range) return false;
        XMFLOAT3 toPlayerDir = Normalize3(toPlayer, eyeDir);

        float alert = Clamp01(std::max(monsterHeadChaseBlend_, chasePanic_));
        float coneHalfDegrees = Lerp(34.0f, 25.0f, alert);
        if (Dot3(eyeDir, toPlayerDir) < std::cos(coneHalfDegrees * kPi / 180.0f)) return false;
        return maze_.LineClear(mt, ct);
    }

    bool MonsterVisualEncounterPlayer() const {
        if (monsterPreview_) return false;
        return MonsterEyeConeLineOfSightToPlayer();
    }

    float MonsterHeadBobOffset() const {
        float phase = monsterPreview_ ? time_ * 2.35f : monsterHeadBobPhase_;
        float chase = monsterPreview_ ? 0.0f : monsterHeadChaseBlend_;
        float amplitude = Lerp(0.050f, 0.086f, chase);
        float secondary = Lerp(0.006f, 0.014f, chase);
        return std::sin(phase) * amplitude + std::sin(phase * 2.0f + 0.65f) * secondary;
    }

    bool MonsterCuriosityActive() const {
        return monsterCuriosityTimer_ > 0.0f && monsterCuriosityDuration_ > 0.001f;
    }

    float MonsterCuriosityAmount() const {
        if (!MonsterCuriosityActive()) return 0.0f;
        float t = 1.0f - Clamp01(monsterCuriosityTimer_ / monsterCuriosityDuration_);
        return SmoothStep(0.0f, 0.24f, t) * (1.0f - SmoothStep(0.82f, 1.0f, t));
    }

    // Player/camera movement, navigation, attention, chase, and camera-state helpers.
#include "game/player_camera_movement.inl"

    bool MonsterFootprintOpen(const XMFLOAT3& pos) const {
        float visualScale = std::clamp(settings_.monsterScale, 0.35f, 1.35f);
        float radius = std::clamp(0.62f * visualScale, 0.26f, 0.92f);
        const XMFLOAT2 samples[] = {
            {0.0f, 0.0f},
            { radius, 0.0f},
            {-radius, 0.0f},
            {0.0f,  radius},
            {0.0f, -radius},
            { radius * 0.92f,  radius * 0.24f},
            {-radius * 0.92f,  radius * 0.24f},
            { radius * 0.92f, -radius * 0.24f},
            {-radius * 0.92f, -radius * 0.24f},
            { radius * 0.24f,  radius * 0.92f},
            {-radius * 0.24f,  radius * 0.92f},
            { radius * 0.24f, -radius * 0.92f},
            {-radius * 0.24f, -radius * 0.92f},
            { radius * 0.62f,  radius * 0.62f},
            {-radius * 0.62f,  radius * 0.62f},
            { radius * 0.62f, -radius * 0.62f},
            {-radius * 0.62f, -radius * 0.62f}
        };
        for (const XMFLOAT2& s : samples) {
            Tile tile = maze_.TileFromWorld(pos.x + s.x, pos.z + s.y);
            if (!maze_.IsOpen(tile.x, tile.y)) return false;
        }
        return true;
    }

    bool MonsterMoveSegmentOpen(const XMFLOAT3& from, const XMFLOAT3& to) const {
        float dx = to.x - from.x;
        float dz = to.z - from.z;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(2, static_cast<int>(std::ceil(len / (maze_.TileMinimum() * 0.055f))));
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            XMFLOAT3 p{from.x + dx * t, 0.0f, from.z + dz * t};
            if (!MonsterFootprintOpen(p)) return false;
        }
        return true;
    }

    void MoveMonsterToward(const XMFLOAT3& target, float distance) {
        if (distance <= 0.0001f) return;
        XMFLOAT3 start = monster_;
        XMFLOAT3 delta{target.x - start.x, 0.0f, target.z - start.z};
        float len = Length3(delta);
        if (len <= 0.0001f) return;
        XMFLOAT3 directDir = Scale3(delta, 1.0f / len);
        XMFLOAT3 dir = directDir;
        bool erraticChase = monsterChasingVisible_ || monsterRecognizedForChase_ || chaseMemoryTimer_ > 0.0f;
        if (erraticChase && len > maze_.TileMinimum() * 0.24f) {
            XMFLOAT3 lateral{dir.z, 0.0f, -dir.x};
            float burst = std::pow(std::max(0.0f, std::sin(time_ * 2.9f + monster_.x * 0.37f - monster_.z * 0.21f)), 6.0f);
            float twitch = std::sin(time_ * 8.7f + monster_.z * 0.53f) * 0.18f +
                std::sin(time_ * 15.4f + monster_.x * 0.29f) * 0.07f;
            float chaseBlend = std::max(monsterHeadChaseBlend_, monsterChasingVisible_ ? 0.72f : 0.36f);
            dir = Normalize3(Add3(dir, Scale3(lateral, twitch * (0.35f + burst * 0.65f) * chaseBlend)), dir);
        }
        XMFLOAT3 avoid = MonsterSelfAvoidanceVector(Add3(start, Scale3(dir, std::min(distance, maze_.TileMinimum() * 0.24f))));
        float avoidLen = Length3(avoid);
        if (avoidLen > 0.001f) {
            float avoidWeight = std::min(0.72f, avoidLen * 0.58f);
            dir = Normalize3(Add3(dir, Scale3(Scale3(avoid, 1.0f / avoidLen), avoidWeight)), dir);
        }
        float remaining = std::min(distance, len);
        int steps = std::max(1, static_cast<int>(std::ceil(remaining / (maze_.TileMinimum() * 0.055f))));
        XMFLOAT3 pos = start;
        bool moved = false;
        for (int i = 0; i < steps; ++i) {
            float step = remaining / static_cast<float>(steps);
            XMFLOAT3 next{pos.x + dir.x * step, 0.0f, pos.z + dir.z * step};
            if (!MonsterFootprintOpen(next)) {
                XMFLOAT3 slideX{pos.x + dir.x * step, 0.0f, pos.z};
                XMFLOAT3 slideZ{pos.x, 0.0f, pos.z + dir.z * step};
                XMFLOAT3 directNext{pos.x + directDir.x * step, 0.0f, pos.z + directDir.z * step};
                bool preferX = std::abs(delta.x) > std::abs(delta.z);
                bool movedSlide = false;
                if (preferX && MonsterFootprintOpen(slideX)) {
                    pos = slideX;
                    movedSlide = true;
                } else if (!preferX && MonsterFootprintOpen(slideZ)) {
                    pos = slideZ;
                    movedSlide = true;
                } else if (MonsterFootprintOpen(directNext)) {
                    pos = directNext;
                    movedSlide = true;
                } else if (MonsterFootprintOpen(preferX ? slideZ : slideX)) {
                    pos = preferX ? slideZ : slideX;
                    movedSlide = true;
                }
                if (!movedSlide) {
                    if (!MonsterFootprintOpen(pos)) {
                        XMFLOAT3 center = maze_.WorldCenter(MonsterTile(), 0.0f);
                        if (MonsterFootprintOpen(center)) monster_ = center;
                    }
                    break;
                }
                moved = true;
                continue;
            }
            pos = next;
            moved = true;
        }
        if (moved) {
            monster_ = pos;
            RecordMonsterTrailPoint(monster_);
            float moveYaw = std::atan2(monster_.x - start.x, monster_.z - start.z);
            if (std::isfinite(moveYaw)) {
                monsterYaw_ += AngleWrap(moveYaw - monsterYaw_) * 0.35f;
            }
        }
    }

    bool ValidMonsterTile(Tile t) const {
        return maze_.IsOpen(t.x, t.y);
    }

    void ClearMonsterPath() {
        monsterPath_.clear();
        monsterPathIndex_ = 0;
        monsterRepath_ = 0.0f;
    }

    void RecordMonsterTrailPoint(const XMFLOAT3& p) {
        XMFLOAT3 head{p.x, 0.0f, p.z};
        float minStep = maze_.TileMinimum() * 0.018f;
        if (!monsterTrail_.empty()) {
            float dx = head.x - monsterTrail_.front().x;
            float dz = head.z - monsterTrail_.front().z;
            if (dx * dx + dz * dz < minStep * minStep) return;
        }
        monsterTrail_.insert(monsterTrail_.begin(), head);
        if (monsterTrail_.size() > 256) monsterTrail_.resize(256);
    }

    XMFLOAT3 MonsterSelfAvoidanceVector(const XMFLOAT3& candidate) const {
        if (monsterTrail_.size() < 10) return {0.0f, 0.0f, 0.0f};
        float spacing = MonsterBodySpacing();
        float ignoreDistance = std::max(maze_.TileMinimum() * 0.82f, spacing * 4.0f);
        float bodyLength = MonsterBodyLengthMeters();
        float repelRadius = std::max(maze_.TileMinimum() * 0.34f, settings_.monsterScale * 0.62f);
        float repelRadiusSq = repelRadius * repelRadius;
        XMFLOAT3 repel{0.0f, 0.0f, 0.0f};
        for (float d = ignoreDistance; d <= bodyLength; d += spacing * 1.35f) {
            XMFLOAT3 sample = MonsterTrailSample(d);
            float dx = candidate.x - sample.x;
            float dz = candidate.z - sample.z;
            float distSq = dx * dx + dz * dz;
            if (distSq <= 0.0001f || distSq > repelRadiusSq) continue;
            float dist = std::sqrt(distSq);
            float weight = 1.0f - dist / repelRadius;
            weight *= weight;
            repel.x += (dx / dist) * weight;
            repel.z += (dz / dist) * weight;
        }
        return repel;
    }

    void SetMonsterGoal(Tile goal, bool force = false) {
        if (!ValidMonsterTile(goal)) return;
        if (!MonsterGoalFarEnough(goal)) return;
        if (force || !(goal == monsterGoal_)) {
            monsterGoal_ = goal;
            ClearMonsterPath();
        }
    }

    bool MonsterCanSeePlayer() const {
        if (MonsterIgnoresPlayer()) return false;
        return MonsterEyeConeLineOfSightToPlayer();
    }

    void UpdateMonsterHeadAnimation(float dt, bool seesPlayer) {
        dt = std::clamp(dt, 0.0f, 0.10f);
        Tile mt = MonsterTile();
        bool validTile = ValidMonsterTile(mt);
        bool openScanSpace = validTile &&
            (maze_.OpenNeighborCount(mt) >= 3 || maze_.LocalOpenCount(mt, 2) >= 14);
        bool activeChase = seesPlayer || monsterChasingVisible_ || monsterHasLastKnown_ ||
            monsterRecognizedForChase_ || chaseMemoryTimer_ > 0.0f || chasePanic_ > 0.08f;
        float curiosity = MonsterCuriosityAmount();

        monsterCanSeePlayerNow_ = seesPlayer;
        monsterHeadChaseBlend_ += ((activeChase ? 1.0f : 0.0f) - monsterHeadChaseBlend_) *
            std::min(1.0f, dt * (activeChase ? 2.35f : 1.00f));
        monsterHeadLockAmount_ += ((seesPlayer ? 1.0f : 0.0f) - monsterHeadLockAmount_) *
            std::min(1.0f, dt * (seesPlayer ? 1.80f : 1.15f));

        float bobSpeed = Lerp(2.35f, 7.85f, monsterHeadChaseBlend_);
        monsterHeadBobPhase_ += dt * bobSpeed;
        if (monsterHeadBobPhase_ > kPi * 64.0f) {
            monsterHeadBobPhase_ = std::fmod(monsterHeadBobPhase_, kPi * 2.0f);
        }

        float scanRate = seesPlayer ? 0.0f : (openScanSpace ? 0.54f : 0.22f);
        monsterHeadScanPhase_ += dt * scanRate;
        if (monsterHeadScanPhase_ > kPi * 64.0f) {
            monsterHeadScanPhase_ = std::fmod(monsterHeadScanPhase_, kPi * 2.0f);
        }

        float yawRange = openScanSpace ? 0.62f : 0.25f;
        float scan = monsterHeadScanPhase_;
        float glideYaw = std::sin(scan) * yawRange +
            std::sin(scan * 0.43f + 1.1f) * yawRange * 0.24f;
        float glidePitch = std::sin(scan * 0.71f + 0.4f) * (openScanSpace ? 0.095f : 0.040f) - 0.024f;
        float focusWindow = std::pow(std::max(0.0f, std::sin(scan * 0.38f + 2.2f)), 18.0f);
        float focusSide = Rand01(static_cast<int>(std::floor(scan * 0.19f)) + mt.x * 11, mt.y * 17 + 991, runtimeSeed_) > 0.5f ? 1.0f : -1.0f;
        float focusYaw = yawRange * focusSide * (0.72f + Rand01(mt.x + 103, mt.y + 211, runtimeSeed_) * 0.42f);
        float focusPitch = -0.045f + Rand01(mt.x + 307, mt.y + 409, runtimeSeed_) * 0.12f;
        float jitterBurst = std::pow(std::max(0.0f, std::sin(scan * 2.9f + 0.7f)), 34.0f);
        float microJitter = (std::sin(time_ * 19.0f + monster_.x * 0.7f) * 0.026f +
            std::sin(time_ * 31.0f + monster_.z * 0.5f) * 0.012f) * jitterBurst;
        float curiosityYaw = std::sin(time_ * 3.4f) * 0.16f + std::sin(time_ * 5.1f + 0.7f) * 0.045f;
        float curiosityPitch = -0.10f + std::sin(time_ * 2.2f + 0.3f) * 0.025f;
        float targetYaw = seesPlayer ? Lerp(0.0f, curiosityYaw, curiosity) : Lerp(glideYaw, focusYaw, focusWindow) + microJitter;
        float targetPitch = seesPlayer ? Lerp(0.0f, curiosityPitch, curiosity) : Lerp(glidePitch, focusPitch, focusWindow) +
            std::sin(time_ * 23.0f) * 0.008f * jitterBurst;
        float response = seesPlayer ? Lerp(2.10f, 3.20f, curiosity) : (openScanSpace ? Lerp(0.68f, 4.35f, focusWindow + jitterBurst * 0.28f) : Lerp(0.52f, 3.25f, focusWindow));
        monsterHeadYawOffset_ += AngleWrap(targetYaw - monsterHeadYawOffset_) *
            std::min(1.0f, dt * response);
        monsterHeadPitchOffset_ += (targetPitch - monsterHeadPitchOffset_) *
            std::min(1.0f, dt * response);
    }

    bool MonsterReachedTile(Tile t) const {
        if (!(MonsterTile() == t)) return false;
        XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
        float dx = center.x - monster_.x;
        float dz = center.z - monster_.z;
        float tile = maze_.TileMinimum();
        return dx * dx + dz * dz < tile * tile * 0.035f;
    }

    bool MonsterGoalFarEnough(Tile goal) const {
        if (!ValidMonsterTile(goal)) return false;
        XMFLOAT3 center = maze_.WorldCenter(goal, 0.0f);
        float dx = center.x - monster_.x;
        float dz = center.z - monster_.z;
        float minDist = std::max(maze_.TileMinimum() * 0.82f, 0.70f * std::clamp(settings_.monsterScale, 0.35f, 1.35f));
        return dx * dx + dz * dz >= minDist * minDist;
    }

    void AlertMonsterToSound(const XMFLOAT3& pos) {
        if (MonsterIgnoresPlayer()) return;
        Tile sound = maze_.TileFromWorld(pos.x, pos.z);
        if (!ValidMonsterTile(sound)) return;
        if (!MonsterGoalFarEnough(sound)) return;
        monsterSoundTile_ = sound;
        monsterHasSound_ = true;
        monsterHasLastKnown_ = false;
        monsterChasingVisible_ = false;
        monsterSearchTimer_ = 0.0f;
        monsterRoamTimer_ = 0.0f;
        SetMonsterGoal(sound, true);
    }

    void AlertMonsterToPlayerTrigger(const XMFLOAT3& fallbackPos) {
        if (MonsterIgnoresPlayer()) return;
        Tile player = CameraTile();
        if (ValidMonsterTile(player)) {
            XMFLOAT3 ping = maze_.WorldCenter(player, 0.0f);
            AlertMonsterToSound(ping);
        } else {
            AlertMonsterToSound(fallbackPos);
        }
    }

    int MonsterSoundWallBlocksBetween(XMFLOAT3 from, XMFLOAT3 to) const {
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

    bool MonsterPassesSoundObstructionChance(const PlayerAudibleSoundPulse& pulse) const {
        XMFLOAT3 monsterPos{monster_.x, 0.08f, monster_.z};
        int wallBlocks = MonsterSoundWallBlocksBetween(monsterPos, pulse.pos);
        float chance = std::pow(0.5f, static_cast<float>(std::clamp(wallBlocks, 0, 8)));
        if (chance >= 0.999f) return true;
        float ageT = pulse.life > 0.001f ? Clamp01(pulse.age / pulse.life) : 0.0f;
        float roll = Rand01(
            static_cast<int>(std::floor(pulse.pos.x * 31.0f + pulse.pos.z * 17.0f + pulse.radius * 11.0f)),
            static_cast<int>(std::floor(time_ * 60.0f + ageT * 997.0f)),
            runtimeSeed_ ^ 0x4F1BBCDCu);
        return roll <= chance;
    }

    void ProcessPlayerAudibleSoundsForMonster(bool seesPlayer) {
        if (MonsterIgnoresPlayer()) return;
        for (PlayerAudibleSoundPulse& pulse : playerAudibleSoundPulses_) {
            if (pulse.processedByMonster || pulse.radius <= 0.01f) continue;
            pulse.processedByMonster = true;
            float dx = monster_.x - pulse.pos.x;
            float dz = monster_.z - pulse.pos.z;
            if (dx * dx + dz * dz > pulse.radius * pulse.radius) continue;
            if (!MonsterGoalFarEnough(maze_.TileFromWorld(pulse.pos.x, pulse.pos.z))) continue;
            if (!MonsterPassesSoundObstructionChance(pulse)) continue;
            pulse.heardByMonster = true;
            monsterHeardPlayerNow_ = true;
            if (!seesPlayer) {
                AlertMonsterToSound(pulse.pos);
            }
        }
    }

    Tile ChooseMonsterRoamTile(Tile from) {
        if (RandRange(0.0f, 1.0f) < 0.42f) {
            std::vector<Tile> neighbors = maze_.Neighbors(from);
            std::vector<Tile> choices;
            choices.reserve(neighbors.size());
            XMFLOAT3 forward{std::sin(monsterYaw_), 0.0f, std::cos(monsterYaw_)};
            for (Tile n : neighbors) {
                if (!ValidMonsterTile(n) || n == maze_.start) continue;
                XMFLOAT3 nc = maze_.WorldCenter(n, 0.0f);
                XMFLOAT3 fc = maze_.WorldCenter(from, 0.0f);
                XMFLOAT3 dir = Normalize3(Sub3(nc, fc), forward);
                if (RandRange(0.0f, 1.0f) < 0.52f || Dot3(dir, forward) < 0.35f) choices.push_back(n);
            }
            if (!choices.empty()) return choices[static_cast<size_t>(rng_() % choices.size())];
        }

        Tile best = from;
        float bestScore = -1.0e9f;
        for (int attempt = 0; attempt < 56; ++attempt) {
            Tile t{
                1 + static_cast<int>(rng_() % std::max(1, maze_.w - 2)),
                1 + static_cast<int>(rng_() % std::max(1, maze_.h - 2))
            };
            if (!ValidMonsterTile(t) || t == maze_.start) continue;
            int pathLength = maze_.PathLength(from, t, 7);
            if (pathLength < 7) continue;
            float cameraSeparation = TileDistanceSq(t, CameraTile());
            float score = static_cast<float>(pathLength) * 1.25f
                + static_cast<float>(maze_.LocalOpenCount(t, 2)) * 3.0f
                + std::min(cameraSeparation, 180.0f) * 0.18f
                + RandRange(0.0f, 24.0f);
            if (maze_.LineClear(t, CameraTile())) score -= 35.0f;
            if (score > bestScore) {
                bestScore = score;
                best = t;
            }
        }

        if (best == from) {
            std::vector<Tile> neighbors = maze_.Neighbors(from);
            if (!neighbors.empty()) {
                best = neighbors[static_cast<size_t>(rng_() % neighbors.size())];
            }
        }
        return best;
    }

    void UpdateDebugMonsterWalk(float dt) {
        if (gDebugHideMonster) return;
        dt = std::clamp(dt, 0.0f, 0.10f);
        int tiles = std::clamp(gDebugSliceTiles, 1, 5);
        float ox = -static_cast<float>(maze_.w) * maze_.tileW * 0.5f;
        float oz = -static_cast<float>(maze_.h) * maze_.tileD * 0.5f;
        float centerX = ox + (1.0f + static_cast<float>(tiles) * 0.5f) * maze_.tileW;
        float centerZ = oz + (1.0f + static_cast<float>(tiles) * 0.5f) * maze_.tileD;
        float margin = maze_.TileMinimum() * 0.62f;
        float spanX = std::max(maze_.TileMinimum() * 0.14f, static_cast<float>(tiles) * maze_.tileW * 0.5f - margin);
        float spanZ = std::max(maze_.TileMinimum() * 0.14f, static_cast<float>(tiles) * maze_.tileD * 0.5f - margin);

        if (!maze_.IsOpen(MonsterTile().x, MonsterTile().y) ||
            Length3(Sub3(monster_, {centerX, 0.0f, centerZ})) > std::max(spanX, spanZ) * 3.5f + maze_.TileAverage()) {
            monster_ = {centerX, 0.0f, centerZ};
            monsterTrail_.clear();
            monsterLimbAnchors_.clear();
            monsterHandprints_.clear();
            for (int i = 0; i < 96; ++i) {
                float back = static_cast<float>(i) * maze_.TileMinimum() * 0.075f;
                monsterTrail_.push_back({monster_.x - std::sin(monsterYaw_) * back, 0.0f, monster_.z - std::cos(monsterYaw_) * back});
            }
        }

        float t = time_ + static_cast<float>(runtimeSeed_ & 1023u) * 0.013f;
        float wanderX = std::sin(t * 0.43f + std::sin(t * 0.17f) * 1.4f) * 0.72f +
            std::sin(t * 1.07f + 1.9f) * 0.22f;
        float wanderZ = std::cos(t * 0.37f + std::sin(t * 0.13f + 0.8f) * 1.7f) * 0.70f +
            std::sin(t * 0.91f - 0.4f) * 0.24f;
        XMFLOAT3 target{
            centerX + std::clamp(wanderX, -1.0f, 1.0f) * spanX,
            0.0f,
            centerZ + std::clamp(wanderZ, -1.0f, 1.0f) * spanZ
        };

        float stopPulse = std::pow(std::max(0.0f, std::sin(t * 0.53f + 1.2f)), 18.0f);
        float lurchPulse = std::pow(std::max(0.0f, std::sin(t * 1.31f - 0.6f)), 5.0f);
        float speed = settings_.monsterSpeed * Lerp(0.58f, 2.15f, lurchPulse) * (1.0f - stopPulse * 0.92f);
        monsterRoamBurstTimer_ = std::max(0.0f, 0.75f - stopPulse);
        monsterHeadChaseBlend_ += (0.0f - monsterHeadChaseBlend_) * std::min(1.0f, dt * 2.2f);
        monsterHeadLockAmount_ += (0.0f - monsterHeadLockAmount_) * std::min(1.0f, dt * 2.0f);
        monsterCanSeePlayerNow_ = false;
        monsterChasingVisible_ = false;
        monsterHasSound_ = false;
        monsterHasLastKnown_ = false;
        monsterRecognizedForChase_ = false;

        UpdateMonsterHeadAnimation(dt, false);
        if (stopPulse < 0.84f) {
            float dist = Length3(Sub3(target, monster_));
            MoveMonsterToward(target, std::min(dist, speed * dt));
        } else {
            float turn = std::sin(t * 2.4f) * 0.46f + std::sin(t * 4.1f + 0.7f) * 0.18f;
            monsterYaw_ += turn * dt;
            RecordMonsterTrailPoint(monster_);
        }
    }

    void UpdateMonster(float dt) {
        monsterRepath_ -= dt;
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        monsterHeardPlayerNow_ = false;
        if (!maze_.IsOpen(mt.x, mt.y)) {
            monster_ = maze_.WorldCenter(maze_.exit, 0.0f);
            monsterTrail_.clear();
            monsterLimbAnchors_.clear();
            monsterHandprints_.clear();
            RecordMonsterTrailPoint(monster_);
            ClearMonsterPath();
            UpdateMonsterHeadAnimation(dt, false);
            return;
        }

        bool seesPlayer = MonsterCanSeePlayer();
        if (MonsterIgnoresPlayer()) {
            seesPlayer = false;
            monsterHasSound_ = false;
            monsterHasLastKnown_ = false;
            monsterChasingVisible_ = false;
            monsterRecognizedForChase_ = false;
            monsterCuriosityTimer_ = 0.0f;
            monsterCuriosityDuration_ = 0.0f;
            chaseMemoryTimer_ = 0.0f;
        }
        ProcessPlayerAudibleSoundsForMonster(seesPlayer);
        if (seesPlayer) {
            monsterRoamPauseTimer_ = 0.0f;
            monsterRoamBurstTimer_ = 0.0f;
            monsterChasingVisible_ = true;
            monsterHasLastKnown_ = true;
            monsterLastKnownTile_ = ct;
            monsterSearchTimer_ = 3.2f;
            SetMonsterGoal(ct);
        } else if (monsterChasingVisible_) {
            monsterChasingVisible_ = false;
            if (monsterHasLastKnown_) {
                monsterSearchTimer_ = RandRange(1.35f, 2.85f);
                SetMonsterGoal(monsterLastKnownTile_, true);
            }
        } else if (monsterHasLastKnown_) {
            monsterRoamPauseTimer_ = 0.0f;
            SetMonsterGoal(monsterLastKnownTile_);
            if (MonsterReachedTile(monsterLastKnownTile_)) {
                monsterSearchTimer_ -= dt;
                if (monsterSearchTimer_ <= 0.0f) {
                    monsterHasLastKnown_ = false;
                    monsterGoal_ = {-1000, -1000};
                    ClearMonsterPath();
                }
            }
        } else if (monsterHasSound_) {
            monsterRoamPauseTimer_ = 0.0f;
            SetMonsterGoal(monsterSoundTile_);
            if (MonsterReachedTile(monsterSoundTile_)) {
                monsterHasSound_ = false;
                monsterGoal_ = {-1000, -1000};
                monsterRoamTimer_ = 0.0f;
                monsterRoamPauseTimer_ = RandRange(0.75f, 2.20f);
                ClearMonsterPath();
            }
        } else {
            monsterRoamTimer_ -= dt;
            if (monsterRoamPauseTimer_ > 0.0f) {
                monsterRoamPauseTimer_ -= dt;
                monsterGoal_ = {-1000, -1000};
                ClearMonsterPath();
            } else if (!ValidMonsterTile(monsterRoamTile_) || MonsterReachedTile(monsterRoamTile_) || monsterRoamTimer_ <= 0.0f) {
                bool reachedRoam = ValidMonsterTile(monsterRoamTile_) && MonsterReachedTile(monsterRoamTile_);
                if (reachedRoam && RandRange(0.0f, 1.0f) < 0.28f) {
                    monsterRoamPauseTimer_ = RandRange(0.45f, RandRange(0.90f, 1.85f));
                    monsterRoamBurstTimer_ = 0.0f;
                    monsterGoal_ = {-1000, -1000};
                    ClearMonsterPath();
                } else {
                    monsterRoamTile_ = ChooseMonsterRoamTile(mt);
                    monsterRoamTimer_ = RandRange(3.20f, 9.80f);
                    monsterRoamBurstTimer_ = RandRange(0.12f, RandRange(0.34f, 0.82f));
                    if (RandRange(0.0f, 1.0f) < 0.42f) {
                        monsterRoamBurstTimer_ = 0.0f;
                    }
                    SetMonsterGoal(monsterRoamTile_, true);
                }
            } else {
                SetMonsterGoal(monsterRoamTile_);
            }
        }

        UpdateMonsterHeadAnimation(dt, seesPlayer);
        bool passiveRoam = !seesPlayer && !monsterHasLastKnown_ && !monsterHasSound_;
        if (passiveRoam && monsterRoamPauseTimer_ > 0.0f) {
            float turn = (std::sin(time_ * 1.37f + monster_.x * 0.21f) * 0.78f +
                std::sin(time_ * 2.21f - monster_.z * 0.17f) * 0.34f);
            monsterYaw_ += turn * dt * 0.72f;
            return;
        }
        if (MonsterCuriosityActive()) return;
        if (!ValidMonsterTile(monsterGoal_)) return;

        bool needPath = monsterRepath_ <= 0.0f || monsterPathIndex_ >= monsterPath_.size()
            || monsterPath_.empty() || !(monsterPath_.back() == monsterGoal_);
        if (needPath) {
            monsterPath_ = maze_.Path(mt, monsterGoal_);
            monsterPathIndex_ = monsterPath_.size() > 1 ? 1 : 0;
            monsterRepath_ = seesPlayer ? 0.22f : (monsterHasLastKnown_ ? 0.42f : 0.95f);
            if (monsterPath_.empty()) {
                monsterHasSound_ = false;
                monsterHasLastKnown_ = false;
                monsterRoamTimer_ = 0.0f;
                monsterGoal_ = {-1000, -1000};
                return;
            }
        }
        if (monsterPathIndex_ < monsterPath_.size()) {
            XMFLOAT3 target = maze_.WorldCenter(monsterPath_[monsterPathIndex_], 0.0f);
            XMFLOAT3 tileCenter = maze_.WorldCenter(mt, 0.0f);
            if (!MonsterMoveSegmentOpen(monster_, target)) {
                target = tileCenter;
                monsterPathIndex_ = 0;
                monsterRepath_ = 0.0f;
            }
            float dx = target.x - monster_.x;
            float dz = target.z - monster_.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < std::max(0.18f, maze_.TileMinimum() * 0.16f)) {
                bool passiveRoamStep = passiveRoam && !monsterPath_.empty();
                ++monsterPathIndex_;
                if (passiveRoamStep) {
                    float stopChance = monsterRoamBurstTimer_ > 0.0f ? 0.12f : 0.045f;
                    if (RandRange(0.0f, 1.0f) < stopChance) {
                        monsterRoamPauseTimer_ = RandRange(0.35f, RandRange(0.75f, 1.55f));
                        monsterRoamBurstTimer_ = 0.0f;
                        monsterRoamTimer_ = 0.0f;
                        monsterGoal_ = {-1000, -1000};
                        ClearMonsterPath();
                        return;
                    }
                    if (RandRange(0.0f, 1.0f) < 0.14f) {
                        monsterRoamBurstTimer_ = std::max(monsterRoamBurstTimer_, RandRange(0.10f, 0.36f));
                    }
                }
                if (monsterPathIndex_ == 1) {
                    monsterRepath_ = 0.0f;
                }
            } else {
                float creepSurge = 0.82f + std::pow(std::max(0.0f, std::sin(time_ * 0.47f + monster_.x * 0.13f + monster_.z * 0.19f)), 9.0f) * 0.68f;
                float speed = 1.08f * settings_.monsterSpeed * creepSurge;
                if (seesPlayer) {
                    float proximity = Clamp01((7.2f - MonsterDistance()) / 6.2f);
                    float impulseSeed = Rand01(mt.x * 43 + mt.y * 59 + static_cast<int>(time_ * 2.1f), 1409, runtimeSeed_);
                    float impulse = std::pow(std::max(0.0f,
                        std::sin(time_ * Lerp(2.1f, 4.4f, impulseSeed) + monster_.x * 0.41f - monster_.z * 0.23f)), Lerp(2.7f, 7.5f, impulseSeed));
                    float stutter = Lerp(0.58f, 1.78f, impulse);
                    speed = Lerp(2.20f, 3.65f, proximity) * settings_.monsterSprintSpeed * (0.88f + creepSurge * 0.12f) * stutter;
                } else if (monsterHasLastKnown_) {
                    float searchSeed = Rand01(mt.x * 23 + mt.y * 71 + static_cast<int>(time_ * 1.4f), 1423, runtimeSeed_);
                    float searchPulse = std::pow(std::max(0.0f,
                        std::sin(time_ * Lerp(1.2f, 3.1f, searchSeed) + monster_.z * 0.35f)), Lerp(2.5f, 6.0f, searchSeed));
                    speed = 2.10f * settings_.monsterSprintSpeed * Lerp(0.62f, 1.46f, searchPulse) * (0.90f + creepSurge * 0.16f);
                } else if (monsterHasSound_) {
                    speed = 2.35f * settings_.monsterSprintSpeed * (0.92f + creepSurge * 0.16f);
                } else {
                    float pulseSeed = Rand01(mt.x * 19 + mt.y * 37 + static_cast<int>(monsterPathIndex_) * 11, 1289, runtimeSeed_);
                    float lizardPulse = 0.5f + 0.5f *
                        std::sin(time_ * Lerp(0.55f, 1.05f, pulseSeed) + monster_.x * 0.19f + monster_.z * 0.17f);
                    lizardPulse = SmoothStep(0.0f, 1.0f, Clamp01(lizardPulse));
                    float burst = monsterRoamBurstTimer_ > 0.0f ? SmoothStep(0.0f, 1.0f, Clamp01(monsterRoamBurstTimer_ / 0.82f)) : 0.0f;
                    float burstNoise = Rand01(mt.x * 17 + mt.y * 31 + static_cast<int>(time_ * 1.7f), 1297, runtimeSeed_);
                    float slowFlow = Lerp(0.72f, 1.12f, lizardPulse);
                    float rush = Lerp(1.75f, 2.75f, burstNoise);
                    speed = settings_.monsterSpeed * Lerp(slowFlow, rush, burst);
                    monsterRoamBurstTimer_ = std::max(0.0f, monsterRoamBurstTimer_ - dt);
                    if (monsterRoamBurstTimer_ <= 0.0f && burst > 0.35f && RandRange(0.0f, 1.0f) < 0.05f) {
                        monsterRoamPauseTimer_ = RandRange(0.28f, 0.72f);
                    }
                }
                MoveMonsterToward(target, std::min(dist, speed * dt));
            }
        }
    }
