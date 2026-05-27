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

    float MonsterHeadBobOffset() const {
        float phase = monsterPreview_ ? time_ * 2.35f : monsterHeadBobPhase_;
        float chase = monsterPreview_ ? 0.0f : monsterHeadChaseBlend_;
        float amplitude = Lerp(0.050f, 0.086f, chase);
        float secondary = Lerp(0.006f, 0.014f, chase);
        return std::sin(phase) * amplitude + std::sin(phase * 2.0f + 0.65f) * secondary;
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
        XMFLOAT3 dir = Scale3(delta, 1.0f / len);
        float remaining = std::min(distance, len);
        int steps = std::max(1, static_cast<int>(std::ceil(remaining / (maze_.TileMinimum() * 0.055f))));
        XMFLOAT3 pos = start;
        bool moved = false;
        for (int i = 0; i < steps; ++i) {
            float step = remaining / static_cast<float>(steps);
            XMFLOAT3 next{pos.x + dir.x * step, 0.0f, pos.z + dir.z * step};
            if (!MonsterFootprintOpen(next)) {
                XMFLOAT3 center = maze_.WorldCenter(MonsterTile(), 0.0f);
                if (MonsterFootprintOpen(center)) {
                    monster_ = center;
                    monsterPath_.clear();
                    monsterPathIndex_ = 0;
                    monsterRepath_ = 0.0f;
                }
                break;
            }
            pos = next;
            moved = true;
        }
        if (moved) {
            monster_ = pos;
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

    void SetMonsterGoal(Tile goal, bool force = false) {
        if (!ValidMonsterTile(goal)) return;
        if (force || !(goal == monsterGoal_)) {
            monsterGoal_ = goal;
            ClearMonsterPath();
        }
    }

    bool MonsterCanSeePlayer() const {
        return MonsterLineOfSightToPlayer();
    }

    void UpdateMonsterHeadAnimation(float dt, bool seesPlayer) {
        dt = std::clamp(dt, 0.0f, 0.10f);
        Tile mt = MonsterTile();
        bool validTile = ValidMonsterTile(mt);
        bool openScanSpace = validTile &&
            (maze_.OpenNeighborCount(mt) >= 3 || maze_.LocalOpenCount(mt, 2) >= 14);
        bool activeChase = seesPlayer || monsterChasingVisible_ || monsterHasLastKnown_ ||
            monsterRecognizedForChase_ || chaseMemoryTimer_ > 0.0f || chasePanic_ > 0.08f;

        monsterCanSeePlayerNow_ = seesPlayer;
        monsterHeadChaseBlend_ += ((activeChase ? 1.0f : 0.0f) - monsterHeadChaseBlend_) *
            std::min(1.0f, dt * (activeChase ? 3.4f : 1.25f));
        monsterHeadLockAmount_ += ((seesPlayer ? 1.0f : 0.0f) - monsterHeadLockAmount_) *
            std::min(1.0f, dt * (seesPlayer ? 7.5f : 2.4f));

        float bobSpeed = Lerp(2.35f, 7.85f, monsterHeadChaseBlend_);
        monsterHeadBobPhase_ += dt * bobSpeed;
        if (monsterHeadBobPhase_ > kPi * 64.0f) {
            monsterHeadBobPhase_ = std::fmod(monsterHeadBobPhase_, kPi * 2.0f);
        }

        float scanRate = seesPlayer ? 0.0f : (openScanSpace ? 0.92f : 0.34f);
        monsterHeadScanPhase_ += dt * scanRate;
        if (monsterHeadScanPhase_ > kPi * 64.0f) {
            monsterHeadScanPhase_ = std::fmod(monsterHeadScanPhase_, kPi * 2.0f);
        }

        float yawRange = openScanSpace ? 0.62f : 0.25f;
        float targetYaw = seesPlayer ? 0.0f :
            (std::sin(monsterHeadScanPhase_) * yawRange +
             std::sin(monsterHeadScanPhase_ * 0.43f + 1.1f) * yawRange * 0.24f);
        float targetPitch = seesPlayer ? 0.0f :
            (std::sin(monsterHeadScanPhase_ * 0.71f + 0.4f) * (openScanSpace ? 0.085f : 0.035f) - 0.018f);
        float response = seesPlayer ? 8.0f : (openScanSpace ? 1.75f : 1.05f);
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

    void AlertMonsterToSound(const XMFLOAT3& pos) {
        Tile sound = maze_.TileFromWorld(pos.x, pos.z);
        if (!ValidMonsterTile(sound)) return;
        monsterSoundTile_ = sound;
        monsterHasSound_ = true;
        monsterHasLastKnown_ = false;
        monsterChasingVisible_ = false;
        monsterSearchTimer_ = 0.0f;
        monsterRoamTimer_ = 0.0f;
        SetMonsterGoal(sound, true);
    }

    void AlertMonsterToPlayerTrigger(const XMFLOAT3& fallbackPos) {
        Tile player = CameraTile();
        if (ValidMonsterTile(player)) {
            XMFLOAT3 ping = maze_.WorldCenter(player, 0.0f);
            AlertMonsterToSound(ping);
        } else {
            AlertMonsterToSound(fallbackPos);
        }
    }

    Tile ChooseMonsterRoamTile(Tile from) {
        Tile best = from;
        float bestScore = -1.0e9f;
        for (int attempt = 0; attempt < 56; ++attempt) {
            Tile t{
                1 + static_cast<int>(rng_() % std::max(1, maze_.w - 2)),
                1 + static_cast<int>(rng_() % std::max(1, maze_.h - 2))
            };
            if (!ValidMonsterTile(t) || t == maze_.start) continue;
            std::vector<Tile> path = maze_.Path(from, t);
            if (path.size() < 7) continue;
            float cameraSeparation = TileDistanceSq(t, CameraTile());
            float score = static_cast<float>(path.size()) * 1.25f
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

    void UpdateMonster(float dt) {
        monsterRepath_ -= dt;
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        monsterHeardPlayerNow_ = false;
        if (!maze_.IsOpen(mt.x, mt.y)) {
            monster_ = maze_.WorldCenter(maze_.exit, 0.0f);
            ClearMonsterPath();
            UpdateMonsterHeadAnimation(dt, false);
            return;
        }

        bool seesPlayer = MonsterCanSeePlayer();
        float hearingRadius = std::max(0.0f, playerNoiseRadiusMeters_);
        if (!seesPlayer && hearingRadius > 0.05f && MonsterDistance() <= hearingRadius) {
            monsterHeardPlayerNow_ = true;
            AlertMonsterToSound({camera_.x, 0.0f, camera_.z});
        }
        if (seesPlayer) {
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
            SetMonsterGoal(monsterSoundTile_);
            if (MonsterReachedTile(monsterSoundTile_)) {
                monsterHasSound_ = false;
                monsterGoal_ = {-1000, -1000};
                monsterRoamTimer_ = 0.0f;
                ClearMonsterPath();
            }
        } else {
            monsterRoamTimer_ -= dt;
            if (!ValidMonsterTile(monsterRoamTile_) || MonsterReachedTile(monsterRoamTile_) || monsterRoamTimer_ <= 0.0f) {
                monsterRoamTile_ = ChooseMonsterRoamTile(mt);
                monsterRoamTimer_ = RandRange(4.5f, 10.0f);
                SetMonsterGoal(monsterRoamTile_, true);
            } else {
                SetMonsterGoal(monsterRoamTile_);
            }
        }

        UpdateMonsterHeadAnimation(dt, seesPlayer);
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
            if (dist < 0.12f) {
                ++monsterPathIndex_;
                if (monsterPathIndex_ == 1) {
                    monsterRepath_ = 0.0f;
                }
            } else {
                float speed = 0.82f * settings_.monsterSpeed;
                if (seesPlayer) {
                    float proximity = Clamp01((7.2f - MonsterDistance()) / 6.2f);
                    speed = Lerp(2.20f, 3.65f, proximity) * settings_.monsterSprintSpeed;
                } else if (monsterHasLastKnown_) {
                    speed = 2.10f * settings_.monsterSprintSpeed;
                } else if (monsterHasSound_) {
                    speed = 1.62f * settings_.monsterSpeed;
                }
                MoveMonsterToward(target, std::min(dist, speed * dt));
            }
        }
    }
