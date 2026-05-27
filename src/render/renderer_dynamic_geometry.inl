    void AppendDynamicQuad(std::vector<Vertex>& verts,
                           XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                           XMFLOAT3 normal, XMFLOAT3 tangent, float material) {
        XMFLOAT2 uvs[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
        XMFLOAT3 pos[4] = {a, b, c, d};
        int order[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; ++i) {
            int idx = order[i];
            verts.push_back({pos[idx], normal, tangent, uvs[idx], material});
        }
    }

    void AppendDynamicQuadUV(std::vector<Vertex>& verts,
                             XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                             XMFLOAT3 normal, XMFLOAT3 tangent,
                             XMFLOAT2 auv, XMFLOAT2 buv, XMFLOAT2 cuv, XMFLOAT2 duv,
                             float material) {
        XMFLOAT2 uvs[4] = {auv, buv, cuv, duv};
        XMFLOAT3 pos[4] = {a, b, c, d};
        int order[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; ++i) {
            int idx = order[i];
            verts.push_back({pos[idx], normal, tangent, uvs[idx], material});
        }
    }

    void AppendDynamicBoxAxes(std::vector<Vertex>& verts, XMFLOAT3 center,
                              XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward,
                              XMFLOAT3 half, float material) {
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        auto p = [&](float x, float y, float z) {
            return Add3(center, Add3(Scale3(right, x * half.x), Add3(Scale3(up, y * half.y), Scale3(forward, z * half.z))));
        };
        AppendDynamicQuad(verts, p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right, material);
        AppendDynamicQuad(verts, p( 1, -1, -1), p(-1, -1, -1), p(-1,  1, -1), p( 1,  1, -1), Scale3(forward, -1.0f), Scale3(right, -1.0f), material);
        AppendDynamicQuad(verts, p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f), material);
        AppendDynamicQuad(verts, p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward, material);
        AppendDynamicQuad(verts, p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right, material);
        AppendDynamicQuad(verts, p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right, material);
    }

    void AppendDynamicBox(std::vector<Vertex>& verts, XMFLOAT3 center, XMFLOAT3 half, float yaw, float material) {
        XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
        AppendDynamicBoxAxes(verts, center, right, up, forward, half, material);
    }

    void AppendSegmentBox(std::vector<Vertex>& verts, XMFLOAT3 a, XMFLOAT3 b,
                          float halfWidth, float halfDepth, float material) {
        XMFLOAT3 axis = Sub3(b, a);
        float len = Length3(axis);
        if (len <= 0.001f) return;
        XMFLOAT3 upAxis = Scale3(axis, 1.0f / len);
        XMFLOAT3 ref{0.0f, 1.0f, 0.0f};
        if (std::abs(Dot3(upAxis, ref)) > 0.88f) ref = {1.0f, 0.0f, 0.0f};
        XMFLOAT3 right = Normalize3(Cross3(ref, upAxis), {1.0f, 0.0f, 0.0f});
        XMFLOAT3 forward = Normalize3(Cross3(upAxis, right), {0.0f, 0.0f, 1.0f});
        AppendDynamicBoxAxes(verts, Lerp3(a, b, 0.5f), right, upAxis, forward, {halfWidth, len * 0.5f, halfDepth}, material);
    }

    void AppendDynamicEllipsoid(std::vector<Vertex>& verts, XMFLOAT3 center,
                                XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward,
                                XMFLOAT3 radius, int slices, int stacks, float material) {
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        slices = std::clamp(slices, 6, 32);
        stacks = std::clamp(stacks, 4, 20);

        struct P {
            XMFLOAT3 pos;
            XMFLOAT3 normal;
            XMFLOAT3 tangent;
            XMFLOAT2 uv;
        };
        auto point = [&](int sx, int sy) {
            float u = static_cast<float>(sx) / static_cast<float>(slices);
            float v = static_cast<float>(sy) / static_cast<float>(stacks);
            float theta = u * kPi * 2.0f;
            float phi = -kPi * 0.5f + v * kPi;
            float cp = std::cos(phi);
            float sp = std::sin(phi);
            float ct = std::cos(theta);
            float st = std::sin(theta);

            XMFLOAT3 pos = Add3(center, Add3(Scale3(right, ct * cp * radius.x),
                Add3(Scale3(up, sp * radius.y), Scale3(forward, st * cp * radius.z))));
            XMFLOAT3 n = Normalize3(Add3(Scale3(right, ct * cp / std::max(0.001f, radius.x)),
                Add3(Scale3(up, sp / std::max(0.001f, radius.y)),
                     Scale3(forward, st * cp / std::max(0.001f, radius.z)))), up);
            XMFLOAT3 t = Normalize3(Add3(Scale3(right, -st), Scale3(forward, ct)), right);
            return P{pos, n, t, {u, v}};
        };
        auto tri = [&](const P& a, const P& b, const P& c) {
            verts.push_back({a.pos, a.normal, a.tangent, a.uv, material});
            verts.push_back({b.pos, b.normal, b.tangent, b.uv, material});
            verts.push_back({c.pos, c.normal, c.tangent, c.uv, material});
        };

        for (int y = 0; y < stacks; ++y) {
            for (int x = 0; x < slices; ++x) {
                P a = point(x, y);
                P b = point(x + 1, y);
                P c = point(x + 1, y + 1);
                P d = point(x, y + 1);
                tri(a, b, c);
                tri(a, c, d);
            }
        }
    }

    XMFLOAT3 ActiveSkullRotationDegrees() const {
        return monsterUsingAltSkull_
            ? XMFLOAT3{settings_.monsterAltSkullPitchDegrees, settings_.monsterAltSkullYawDegrees, settings_.monsterAltSkullRollDegrees}
            : XMFLOAT3{settings_.monsterSkullPitchDegrees, settings_.monsterSkullYawDegrees, settings_.monsterSkullRollDegrees};
    }

    XMFLOAT3 RotateSkullLocalVector(XMFLOAT3 v) const {
        XMFLOAT3 degrees = ActiveSkullRotationDegrees();
        float pitch = degrees.x * kPi / 180.0f;
        float yaw = degrees.y * kPi / 180.0f;
        float roll = degrees.z * kPi / 180.0f;

        float cy = std::cos(yaw);
        float sy = std::sin(yaw);
        v = {v.x * cy + v.z * sy, v.y, -v.x * sy + v.z * cy};

        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        v = {v.x, v.y * cp - v.z * sp, v.y * sp + v.z * cp};

        float cr = std::cos(roll);
        float sr = std::sin(roll);
        v = {v.x * cr - v.y * sr, v.x * sr + v.y * cr, v.z};
        return v;
    }

    bool AppendExternalSkullMesh(std::vector<Vertex>& verts, XMFLOAT3 center,
                                 XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward,
                                 float modelXZ, float modelY) {
        if (skullMesh_.empty()) return false;
        if (verts.size() + skullMesh_.size() + 512 > kDynamicVertexCapacity) return false;
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        for (const Vertex& src : skullMesh_) {
            XMFLOAT3 baseLocal = monsterSkullNativeMaskAxes_ ? src.pos : XMFLOAT3{-src.pos.x, src.pos.y, -src.pos.z};
            XMFLOAT3 baseNormal = monsterSkullNativeMaskAxes_ ? src.normal : XMFLOAT3{-src.normal.x, src.normal.y, -src.normal.z};
            XMFLOAT3 baseTangent = monsterSkullNativeMaskAxes_ ? src.tangent : XMFLOAT3{-src.tangent.x, src.tangent.y, -src.tangent.z};
            XMFLOAT3 local = RotateSkullLocalVector(baseLocal);
            XMFLOAT3 nLocal = RotateSkullLocalVector(baseNormal);
            XMFLOAT3 tLocal = RotateSkullLocalVector(baseTangent);
            XMFLOAT3 pos = Add3(center, Add3(Scale3(right, local.x * modelXZ),
                Add3(Scale3(up, local.y * modelY), Scale3(forward, local.z * modelXZ))));
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, nLocal.x),
                Add3(Scale3(up, nLocal.y), Scale3(forward, nLocal.z))), forward);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, tLocal.x),
                Add3(Scale3(up, tLocal.y), Scale3(forward, tLocal.z))), right);
            verts.push_back({pos, normal, tangent, src.uv, src.material});
        }
        return true;
    }

    void AppendDynamicDoor(std::vector<Vertex>& verts) {
        float halfW = 0.60f;
        float halfH = 1.05f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        float angle = exitDoorAngle_;
        XMFLOAT3 right = RotateYVec(exitDoorRight_, angle);
        XMFLOAT3 normal = RotateYVec(exitDoorNormal_, angle);
        XMFLOAT3 center = Add3(exitDoorHinge_, Add3(Scale3(right, halfW), Scale3(normal, 0.012f)));
        AppendDynamicBoxAxes(verts, center, right, up, normal, {halfW, halfH, 0.032f}, 6.0f);

        XMFLOAT3 knobCenter = Add3(center, OrientedOffset(right, up, normal, halfW * 0.63f, -0.08f, 0.070f));
        XMFLOAT3 kr = Scale3(right, 0.050f);
        XMFLOAT3 ku = Scale3(up, 0.050f);
        AppendDynamicQuad(verts,
            Add3(knobCenter, Add3(Scale3(kr, -1.0f), Scale3(ku, -1.0f))),
            Add3(knobCenter, Add3(kr, Scale3(ku, -1.0f))),
            Add3(knobCenter, Add3(kr, ku)),
            Add3(knobCenter, Add3(Scale3(kr, -1.0f), ku)),
            normal, right, 10.0f);
    }

    void AppendMenuDoorwayLight(std::vector<Vertex>& transparentVerts) {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return;
        float openT = SmoothStep(0.05f, 1.12f, exitDoorAngle_);
        if (openT <= 0.001f) return;

        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 right = Normalize3(exitDoorRight_, {1.0f, 0.0f, 0.0f});
        XMFLOAT3 inward = Normalize3(exitDoorNormal_, {0.0f, 0.0f, 1.0f});
        XMFLOAT3 behind = Scale3(inward, -1.0f);
        XMFLOAT3 start = Add3(exitDoorCenter_, Add3(Scale3(behind, 0.10f), {0.0f, 0.04f, 0.0f}));
        XMFLOAT3 room = Add3(exitDoorCenter_, Add3(Scale3(inward, 2.45f), {0.0f, -0.03f, 0.0f}));
        float openMat = 19.58f + openT * 0.22f;

        XMFLOAT3 nearSide = Scale3(right, 0.56f);
        XMFLOAT3 farSide = Scale3(right, 0.82f);
        XMFLOAT3 nearUp = Scale3(up, 1.02f);
        XMFLOAT3 farUp = Scale3(up, 1.12f);
        XMFLOAT3 centerYNear = Add3(start, {0.0f, 1.12f, 0.0f});
        XMFLOAT3 centerYFar = Add3(room, {0.0f, 1.10f, 0.0f});

        AppendDynamicQuadUV(transparentVerts,
            Add3(centerYNear, Add3(Scale3(nearSide, -1.0f), Scale3(nearUp, -1.0f))),
            Add3(centerYNear, Add3(nearSide, Scale3(nearUp, -1.0f))),
            Add3(centerYNear, Add3(nearSide, nearUp)),
            Add3(centerYNear, Add3(Scale3(nearSide, -1.0f), nearUp)),
            inward, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, openMat + 0.018f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(centerYNear, Scale3(nearUp, -1.0f)),
            Add3(centerYFar, Scale3(farUp, -1.0f)),
            Add3(centerYFar, farUp),
            Add3(centerYNear, nearUp),
            right, inward, {0, 1}, {1, 1}, {1, 0}, {0, 0}, openMat);
        AppendDynamicQuadUV(transparentVerts,
            Add3(centerYFar, Add3(Scale3(farSide, -1.0f), Scale3(up, -0.08f))),
            Add3(centerYFar, Add3(farSide, Scale3(up, -0.08f))),
            Add3(centerYNear, Add3(nearSide, Scale3(up, 0.18f))),
            Add3(centerYNear, Add3(Scale3(nearSide, -1.0f), Scale3(up, 0.18f))),
            up, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, openMat + 0.045f);
    }

    void AppendMenuButtonPlaques(std::vector<Vertex>& verts, std::vector<Vertex>& transparentVerts) {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return;
        XMFLOAT3 c = maze_.WorldCenter(maze_.start, 0.0f);
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        MenuPlaquePlacement primaryPlaque = MenuButtonPlacement(0);
        XMFLOAT3 right = primaryPlaque.right;
        XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
        float wallZ = c.z + maze_.tileD * 0.5f;
        float x = primaryPlaque.center.x;
        float bloodTopY = settings_.wallHeightMeters - 0.006f;
        float bloodBottomY = 0.012f;
        float bloodCenterY = (bloodTopY + bloodBottomY) * 0.5f;
        float bloodHalfHeight = std::max(0.40f, (bloodTopY - bloodBottomY) * 0.5f);
        XMFLOAT3 bloodCenter{x, bloodCenterY, wallZ - 0.002f};
        XMFLOAT3 bloodHalfW = Scale3(right, primaryPlaque.halfW * 1.03f);
        XMFLOAT3 bloodHalfH = Scale3(up, bloodHalfHeight);
        constexpr float kMenuWallBloodMaterial = 14.985f;
        constexpr float kMenuPoolBloodMaterial = 14.66f;
        AppendDynamicQuadUV(transparentVerts,
            Add3(bloodCenter, Add3(Scale3(bloodHalfW, -1.0f), Scale3(bloodHalfH, -1.0f))),
            Add3(bloodCenter, Add3(bloodHalfW, Scale3(bloodHalfH, -1.0f))),
            Add3(bloodCenter, Add3(bloodHalfW, bloodHalfH)),
            Add3(bloodCenter, Add3(Scale3(bloodHalfW, -1.0f), bloodHalfH)),
            inward, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, kMenuWallBloodMaterial);
        float ceilingDepth = std::min(maze_.tileD * 0.42f, 0.68f);
        float ceilingY = settings_.wallHeightMeters - 0.003f;
        XMFLOAT3 ceilingCenter{x, ceilingY, wallZ - ceilingDepth * 0.16f};
        XMFLOAT3 ceilingHalfD = Scale3(inward, ceilingDepth * 0.5f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(ceilingCenter, Add3(Scale3(bloodHalfW, -1.0f), Scale3(ceilingHalfD, -1.0f))),
            Add3(ceilingCenter, Add3(bloodHalfW, Scale3(ceilingHalfD, -1.0f))),
            Add3(ceilingCenter, Add3(bloodHalfW, ceilingHalfD)),
            Add3(ceilingCenter, Add3(Scale3(bloodHalfW, -1.0f), ceilingHalfD)),
            Scale3(up, -1.0f), right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, kMenuPoolBloodMaterial);
        float poolDepth = std::min(maze_.tileD * 0.40f, 0.64f);
        XMFLOAT3 poolCenter{x, 0.014f, wallZ - poolDepth * 0.15f};
        XMFLOAT3 poolHalfW = Scale3(right, primaryPlaque.halfW * 0.94f);
        XMFLOAT3 poolHalfD = Scale3(inward, poolDepth * 0.5f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(poolCenter, Add3(Scale3(poolHalfW, -1.0f), poolHalfD)),
            Add3(poolCenter, Add3(poolHalfW, poolHalfD)),
            Add3(poolCenter, Add3(poolHalfW, Scale3(poolHalfD, -1.0f))),
            Add3(poolCenter, Add3(Scale3(poolHalfW, -1.0f), Scale3(poolHalfD, -1.0f))),
            up, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, kMenuPoolBloodMaterial);
        for (int i = 0; i < 3; ++i) {
            bool hover = menuHoverButtonIndex_ == i;
            float material = hover ? 10.72f : 9.70f;
            MenuPlaquePlacement plaque = MenuButtonPlacement(i);
            AppendDynamicBoxAxes(verts, plaque.center, plaque.right, up, plaque.inward, {plaque.halfW, plaque.halfH, 0.034f}, material);
            XMFLOAT3 capCenter = Add3(plaque.center, Add3(Scale3(plaque.right, -plaque.halfW + 0.012f), Scale3(plaque.inward, -0.017f)));
            AppendDynamicBoxAxes(verts, capCenter, plaque.right, up, plaque.inward, {0.018f, plaque.halfH + 0.012f, 0.026f}, 10.0f);
            XMFLOAT3 labelCenter = Add3(plaque.center, Scale3(plaque.inward, 0.036f));
            XMFLOAT3 hw = Scale3(plaque.right, std::min(plaque.halfW * 0.58f, 0.78f));
            XMFLOAT3 hh = Scale3(up, 0.112f);
            float v0 = (static_cast<float>(i) + 0.12f) / 3.0f;
            float v1 = (static_cast<float>(i) + 0.88f) / 3.0f;
            float labelMaterial = hover ? 18.65f : 18.08f;
            AppendDynamicQuadUV(transparentVerts,
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, hh)),
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), hh)),
                plaque.inward, plaque.right, {0.06f, v1}, {0.94f, v1}, {0.94f, v0}, {0.06f, v0}, labelMaterial);
        }
    }

    void AppendMonsterBillboard(std::vector<Vertex>& solidVerts, std::vector<Vertex>& transparentVerts) {
        float modelY = std::clamp(settings_.monsterScale, 0.35f, 1.25f);
        float modelXZ = std::clamp(settings_.monsterScale, 0.35f, 1.35f);
        float dist = MonsterDistance();
        bool canTrackPlayer = !monsterPreview_ && MonsterVisualEncounterPlayer();
        float faceYaw = monsterYaw_;
        if (canTrackPlayer) {
            float cameraYaw = std::atan2(camera_.x - monster_.x, camera_.z - monster_.z);
            float turnIn = 0.06f + SmoothStep(0.0f, 1.0f, monsterHeadLockAmount_) * 0.28f;
            faceYaw += AngleWrap(cameraYaw - faceYaw) * turnIn;
        }

        XMFLOAT3 right{std::cos(faceYaw), 0.0f, -std::sin(faceYaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(faceYaw), 0.0f, std::cos(faceYaw)};
        float hover = 0.22f + std::sin(time_ * 1.55f + monster_.x * 0.07f + monster_.z * 0.05f) * 0.050f;
        auto off = [&](float x, float y, float z) {
            return Add3(monster_, OrientedOffset(right, up, forward, x * modelXZ, y * modelY + hover, z * modelXZ));
        };
        auto box = [&](float x, float y, float z, float hx, float hy, float hz, float material) {
            AppendDynamicBoxAxes(solidVerts, off(x, y, z), right, up, forward,
                {hx * modelXZ, hy * modelY, hz * modelXZ}, material);
        };
        auto seg = [&](XMFLOAT3 a, XMFLOAT3 b, float w, float d, float material) {
            AppendSegmentBox(solidVerts, a, b, w * modelXZ, d * modelXZ, material);
        };

        float twitch = std::sin(time_ * 13.7f + monster_.x * 0.3f) * 0.035f;
        float breathe = std::sin(time_ * 2.3f) * 0.030f;
        float deathHeadLock = deathActive_ ? SmoothStep(0.0f, 0.22f, deathTimer_) : 0.0f;
        constexpr float boneMat = 9.65f;
        constexpr float gutMat = 20.68f;
        constexpr float limbMat = 20.74f;
        constexpr float darkMat = 10.0f;
        constexpr float handprintMat = 25.35f;

        auto surfaceAxes = [&](XMFLOAT3 normal, XMFLOAT3& axisX, XMFLOAT3& axisY) {
            normal = Normalize3(normal, up);
            if (std::abs(normal.y) > 0.82f) {
                axisX = right;
                axisY = Normalize3(Cross3(normal, axisX), forward);
            } else {
                axisX = Normalize3(Cross3(up, normal), right);
                axisY = up;
            }
        };
        auto appendBloodHandprint = [&](const MonsterHandprint& hp) {
            XMFLOAT3 n = Normalize3(hp.normal, up);
            XMFLOAT3 axisX{};
            XMFLOAT3 axisY{};
            surfaceAxes(n, axisX, axisY);
            float w = hp.size * 0.92f;
            float h = hp.size * (1.18f + std::fmod(hp.seed * 23.0f, 0.36f));
            XMFLOAT3 center = Add3(hp.pos, Scale3(n, 0.0035f));
            XMFLOAT3 hx = Scale3(axisX, w);
            XMFLOAT3 hy = Scale3(axisY, h);
            float mat = handprintMat + std::fmod(hp.seed * 0.137f, 0.48f);
            AppendDynamicQuadUV(transparentVerts,
                Add3(center, Add3(Scale3(hx, -1.0f), Scale3(hy, -1.0f))),
                Add3(center, Add3(hx, Scale3(hy, -1.0f))),
                Add3(center, Add3(hx, hy)),
                Add3(center, Add3(Scale3(hx, -1.0f), hy)),
                n, axisX, {0, 1}, {1, 1}, {1, 0}, {0, 0}, mat);
        };
        auto recordHandprint = [&](XMFLOAT3, XMFLOAT3, int, int) {
        };
        monsterHandprints_.clear();

        auto smokeMaterial = [&](float seed) {
            return 11.08f + std::fmod(std::abs(seed), 0.34f);
        };
        auto smokeBand = [&](XMFLOAT3 a, XMFLOAT3 b, float halfA, float halfB, float material) {
            XMFLOAT3 axis = Sub3(b, a);
            float len = Length3(axis);
            if (len <= 0.001f) return;
            XMFLOAT3 axisN = Scale3(axis, 1.0f / len);
            XMFLOAT3 mid = Lerp3(a, b, 0.5f);
            XMFLOAT3 toCam = Normalize3(Sub3(camera_, mid), forward);
            XMFLOAT3 side = Normalize3(Cross3(axisN, toCam), right);
            XMFLOAT3 normal = Normalize3(Cross3(side, axisN), toCam);
            AppendDynamicQuad(transparentVerts,
                Add3(a, Scale3(side, -halfA * modelXZ)),
                Add3(a, Scale3(side, halfA * modelXZ)),
                Add3(b, Scale3(side, halfB * modelXZ)),
                Add3(b, Scale3(side, -halfB * modelXZ)),
                normal, side, material);
        };
        auto smokePuff = [&](float x, float y, float z, float halfW, float halfH, float material) {
            XMFLOAT3 center = off(x, y, z);
            XMFLOAT3 toCam = Normalize3(Sub3(camera_, center), forward);
            XMFLOAT3 side = Normalize3(Cross3(up, toCam), right);
            XMFLOAT3 puffUp = Normalize3(Cross3(toCam, side), up);
            XMFLOAT3 hw = Scale3(side, halfW * modelXZ);
            XMFLOAT3 hh = Scale3(puffUp, halfH * modelY);
            AppendDynamicQuad(transparentVerts,
                Add3(center, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(center, Add3(hw, Scale3(hh, -1.0f))),
                Add3(center, Add3(hw, hh)),
                Add3(center, Add3(Scale3(hw, -1.0f), hh)),
                toCam, side, material);
        };

        auto sampleTrail = [&](float targetDistance) {
            if (monsterTrail_.empty()) return monster_;
            XMFLOAT3 prev = monsterTrail_.front();
            float travelled = 0.0f;
            for (size_t i = 1; i < monsterTrail_.size(); ++i) {
                XMFLOAT3 next = monsterTrail_[i];
                float dx = next.x - prev.x;
                float dz = next.z - prev.z;
                float len = std::sqrt(dx * dx + dz * dz);
                if (travelled + len >= targetDistance && len > 0.001f) {
                    float t = (targetDistance - travelled) / len;
                    return XMFLOAT3{Lerp(prev.x, next.x, t), 0.0f, Lerp(prev.z, next.z, t)};
                }
                travelled += len;
                prev = next;
            }
            float back = std::max(0.0f, targetDistance - travelled);
            return XMFLOAT3{prev.x - std::sin(monsterYaw_) * back, 0.0f, prev.z - std::cos(monsterYaw_) * back};
        };
        struct SurfaceContact {
            XMFLOAT3 point;
            XMFLOAT3 normal;
        };
        auto contactPoint = [&](XMFLOAT3 root, XMFLOAT3 sideDir, XMFLOAT3 tangentDir, XMFLOAT3 upDir, int limbIndex, int cycle) {
            Tile t = maze_.TileFromWorld(root.x, root.z);
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float halfW = std::max(0.12f, maze_.tileW * 0.5f - 0.018f);
            float halfD = std::max(0.12f, maze_.tileD * 0.5f - 0.018f);
            float crawl = static_cast<float>((limbIndex * 37 + cycle * 17) & 1023);
            sideDir = Normalize3(sideDir, right);
            tangentDir = Normalize3(tangentDir, forward);
            upDir = Normalize3(upDir, up);
            float reach = maze_.TileMinimum() * (0.22f + Rand01(limbIndex * 11 + cycle * 3, 719, runtimeSeed_) * 0.15f);
            float sideBias = 0.20f + Rand01(limbIndex * 13 + cycle * 5, 727, runtimeSeed_) * 0.36f;
            float tangentBias = (Rand01(limbIndex * 17 + cycle * 7, 731, runtimeSeed_) - 0.5f) * 1.18f;
            float verticalBias = (Rand01(limbIndex * 19 + cycle * 11, 733, runtimeSeed_) - 0.5f) * 1.34f;
            if (cycle % 5 == 1) verticalBias += 0.72f;
            if (cycle % 7 == 2) verticalBias -= 0.72f;
            XMFLOAT3 radial = Normalize3(Add3(Scale3(sideDir, sideBias),
                Add3(Scale3(tangentDir, tangentBias), Scale3(upDir, verticalBias))), sideDir);
            XMFLOAT3 desired = Add3(root, Scale3(radial, reach));
            desired.y = std::clamp(desired.y, 0.035f, settings_.wallHeightMeters - 0.045f);
            XMFLOAT3 anchorBase = Lerp3(root, desired, 0.24f);
            float jitterA = (Rand01(limbIndex + cycle * 3, 739, runtimeSeed_) - 0.5f) * maze_.TileMinimum() * 0.075f;
            float jitterB = (Rand01(limbIndex + cycle * 5, 743, runtimeSeed_) - 0.5f) * maze_.TileMinimum() * 0.075f;
            float jitterY = (Rand01(limbIndex + cycle * 7, 751, runtimeSeed_) - 0.5f) * maze_.TileMinimum() * 0.055f;
            float surfaceY = std::clamp(anchorBase.y + jitterY, 0.040f, settings_.wallHeightMeters - 0.045f);
            std::array<XMFLOAT3, 6> candidates = {{
                {c.x - halfW, surfaceY, std::clamp(anchorBase.z + jitterA, c.z - halfD, c.z + halfD)},
                {c.x + halfW, surfaceY, std::clamp(anchorBase.z - jitterA, c.z - halfD, c.z + halfD)},
                {std::clamp(anchorBase.x + jitterB, c.x - halfW, c.x + halfW), surfaceY, c.z - halfD},
                {std::clamp(anchorBase.x - jitterB, c.x - halfW, c.x + halfW), surfaceY, c.z + halfD},
                {std::clamp(anchorBase.x + jitterB * 0.5f, c.x - halfW, c.x + halfW), 0.022f, std::clamp(anchorBase.z + jitterA * 0.5f, c.z - halfD, c.z + halfD)},
                {std::clamp(anchorBase.x - jitterB * 0.5f, c.x - halfW, c.x + halfW), settings_.wallHeightMeters - 0.035f, std::clamp(anchorBase.z - jitterA * 0.5f, c.z - halfD, c.z + halfD)}
            }};
            std::array<XMFLOAT3, 6> normals = {{
                {1.0f, 0.0f, 0.0f},
                {-1.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, -1.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, -1.0f, 0.0f}
            }};
            std::array<bool, 6> usable = {{
                !maze_.IsOpen(t.x - 1, t.y),
                !maze_.IsOpen(t.x + 1, t.y),
                !maze_.IsOpen(t.x, t.y - 1),
                !maze_.IsOpen(t.x, t.y + 1),
                true,
                true
            }};
            int surfaceCycle = std::abs((limbIndex * 5 + cycle * 3 + static_cast<int>(crawl)) % 9);
            int preferred = surfaceCycle == 0 ? 5 : (surfaceCycle == 1 ? 4 : -1);
            float bestScore = 1.0e9f;
            XMFLOAT3 best = candidates[4];
            XMFLOAT3 bestNormal = normals[4];
            for (int i = 0; i < 6; ++i) {
                if (!usable[static_cast<size_t>(i)]) continue;
                XMFLOAT3 toRoot = Sub3(candidates[static_cast<size_t>(i)], root);
                float distToRoot = Length3(toRoot);
                float surfaceSideFit = std::max(0.0f, Dot3(normals[static_cast<size_t>(i)], sideDir));
                float score = distToRoot * distToRoot + std::abs(distToRoot - reach) * 0.055f;
                if (distToRoot > reach * 1.28f) {
                    float over = distToRoot - reach * 1.28f;
                    score += over * over * 12.0f;
                }
                if (preferred == i) score *= 0.82f;
                if (i < 4) score *= (0.92f - surfaceSideFit * 0.14f);
                if (score < bestScore) {
                    bestScore = score;
                    best = candidates[static_cast<size_t>(i)];
                    bestNormal = normals[static_cast<size_t>(i)];
                }
            }
            return SurfaceContact{best, bestNormal};
        };
        auto organicSegment = [&](XMFLOAT3 a, XMFLOAT3 b, float radiusA, float radiusB, float material, int sides = 9) {
            XMFLOAT3 axis = Sub3(b, a);
            float len = Length3(axis);
            if (len <= 0.001f) return;
            XMFLOAT3 tangent = Scale3(axis, 1.0f / len);
            XMFLOAT3 ref = std::abs(Dot3(tangent, up)) > 0.86f ? right : up;
            XMFLOAT3 side0 = Normalize3(Cross3(ref, tangent), right);
            XMFLOAT3 side1 = Normalize3(Cross3(tangent, side0), up);
            sides = std::clamp(sides, 6, 18);
            for (int s = 0; s < sides; ++s) {
                float a0 = static_cast<float>(s) / static_cast<float>(sides) * kPi * 2.0f;
                float a1 = static_cast<float>(s + 1) / static_cast<float>(sides) * kPi * 2.0f;
                auto p = [&](XMFLOAT3 base, float angle, float r, float wobbleSeed) {
                    float lump = 1.0f + std::sin(angle * 3.0f + wobbleSeed) * 0.075f +
                        std::sin(angle * 5.0f + wobbleSeed * 0.71f) * 0.035f;
                    return Add3(base, Add3(Scale3(side0, std::cos(angle) * r * lump),
                                           Scale3(side1, std::sin(angle) * r * lump)));
                };
                XMFLOAT3 p0 = p(a, a0, radiusA, time_ * 1.7f + a.x * 0.3f);
                XMFLOAT3 p1 = p(a, a1, radiusA, time_ * 1.7f + a.x * 0.3f);
                XMFLOAT3 p2 = p(b, a1, radiusB, time_ * 1.9f + b.z * 0.3f);
                XMFLOAT3 p3 = p(b, a0, radiusB, time_ * 1.9f + b.z * 0.3f);
                XMFLOAT3 normal = Normalize3(Add3(Scale3(side0, std::cos((a0 + a1) * 0.5f)),
                                                  Scale3(side1, std::sin((a0 + a1) * 0.5f))), side0);
                AppendDynamicQuadUV(solidVerts, p0, p1, p2, p3, normal, tangent,
                    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, material);
            }
        };

        constexpr int maxBodyCount = 36;
        std::array<XMFLOAT3, maxBodyCount> bodyPoints{};
        std::array<XMFLOAT3, maxBodyCount> bodyCenterlinePoints{};
        std::array<float, maxBodyCount> bodyRadii{};
        std::array<XMFLOAT3, maxBodyCount> bodySides{};
        std::array<XMFLOAT3, maxBodyCount> bodyUps{};
        std::array<XMFLOAT3, maxBodyCount> bodyTangents{};
        std::array<float, maxBodyCount> bodyUvV{};
        std::array<float, maxBodyCount> bodyUvShift{};
        float bodyLengthMeters = std::min(10.0f, 3.9f + static_cast<float>(std::max(0, monsterKillCount_)) * 0.75f);
        float bodySpacing = maze_.TileMinimum() * 0.24f;
        int bodyCount = std::clamp(static_cast<int>(std::ceil(bodyLengthMeters / std::max(0.12f, bodySpacing))) + 1, 9, maxBodyCount);
        for (int i = 0; i < bodyCount; ++i) {
            float fi = static_cast<float>(i);
            XMFLOAT3 p = sampleTrail(fi * bodySpacing);
            float taper = 1.0f - fi / static_cast<float>(bodyCount);
            p.y = 0.0f;
            bodyPoints[static_cast<size_t>(i)] = p;
            bodyCenterlinePoints[static_cast<size_t>(i)] = p;
            float peristalsis = 1.0f + std::sin(time_ * 3.85f - fi * 0.91f) * 0.135f;
            bodyRadii[static_cast<size_t>(i)] = (0.20f + taper * 0.32f) * modelXZ * peristalsis;
            bodyUvShift[static_cast<size_t>(i)] = std::fmod(Rand01(i * 41 + static_cast<int>(runtimeSeed_ & 1023), 1709, runtimeSeed_) * 0.47f +
                std::sin(fi * 1.73f + static_cast<float>(runtimeSeed_ & 255) * 0.013f) * 0.08f, 1.0f);
        }
        bodyUvV[0] = static_cast<float>(runtimeSeed_ & 4095) * 0.0017f;
        for (int i = 1; i < bodyCount; ++i) {
            float segmentLen = Length3(Sub3(bodyPoints[static_cast<size_t>(i)], bodyPoints[static_cast<size_t>(i - 1)]));
            float seedStretch = 0.86f + Rand01(i * 53 + static_cast<int>(runtimeSeed_ & 2047), 1723, runtimeSeed_) * 0.48f;
            bodyUvV[static_cast<size_t>(i)] = bodyUvV[static_cast<size_t>(i - 1)] + segmentLen * seedStretch * 0.62f;
        }
        for (int i = 0; i < bodyCount; ++i) {
            XMFLOAT3 p = bodyPoints[static_cast<size_t>(i)];
            XMFLOAT3 prev = i > 0 ? bodyPoints[static_cast<size_t>(i - 1)] : bodyPoints[static_cast<size_t>(i)];
            XMFLOAT3 next = i + 1 < bodyCount ? bodyPoints[static_cast<size_t>(i + 1)] : bodyPoints[static_cast<size_t>(i)];
            XMFLOAT3 tangent = Normalize3(Sub3(prev, next), forward);
            XMFLOAT3 side = Normalize3(Cross3(up, tangent), right);
            bodySides[static_cast<size_t>(i)] = side;
            bodyTangents[static_cast<size_t>(i)] = tangent;
        }
        for (int i = 0; i < bodyCount; ++i) {
            float fi = static_cast<float>(i);
            float taper = 1.0f - fi / static_cast<float>(bodyCount);
            XMFLOAT3 side = bodySides[static_cast<size_t>(i)];
            XMFLOAT3 tangent = bodyTangents[static_cast<size_t>(i)];
            float radius = bodyRadii[static_cast<size_t>(i)];
            struct SurfaceChoice {
                XMFLOAT3 normal;
                XMFLOAT3 center;
                float score;
            };
            XMFLOAT3 p = bodyPoints[static_cast<size_t>(i)];
            Tile tile = maze_.TileFromWorld(p.x, p.z);
            XMFLOAT3 tileCenter = maze_.WorldCenter(tile, 0.0f);
            float halfW = maze_.tileW * 0.5f;
            float halfD = maze_.tileD * 0.5f;
            float localX = p.x - tileCenter.x;
            float localZ = p.z - tileCenter.z;
            float surfaceNoiseA = std::sin(time_ * 0.19f + fi * 0.71f + monster_.x * 0.07f);
            float surfaceNoiseB = std::sin(time_ * 0.13f - fi * 0.53f + monster_.z * 0.09f);
            float surfaceNoiseC = std::sin(time_ * 0.073f + fi * 1.37f + static_cast<float>(runtimeSeed_ & 255) * 0.017f);
            SurfaceChoice best{{0.0f, 1.0f, 0.0f}, Add3(p, {0.0f, radius * 1.05f + 0.045f, 0.0f}),
                0.42f - surfaceNoiseA * 0.16f};
            SurfaceChoice ceiling{{0.0f, -1.0f, 0.0f}, {p.x, settings_.wallHeightMeters - radius * 1.05f - 0.045f, p.z},
                0.52f - surfaceNoiseB * 0.20f};
            if (ceiling.score < best.score) best = ceiling;
            auto considerWall = [&](int dx, int dz, XMFLOAT3 normal, float planeCoord, float distanceToPlane) {
                if (maze_.IsOpen(tile.x + dx, tile.y + dz)) return;
                float wallBand = 0.18f + 0.66f * (0.5f + 0.5f * std::sin(time_ * 0.17f + fi * 0.31f + dx * 1.7f + dz * 2.3f));
                float y = std::clamp(settings_.wallHeightMeters * wallBand,
                    radius * 1.05f + 0.045f,
                    settings_.wallHeightMeters - radius * 1.05f - 0.045f);
                XMFLOAT3 c = p;
                c.y = y;
                if (dx != 0) {
                    c.x = planeCoord + normal.x * (radius * 1.18f + 0.035f);
                } else {
                    c.z = planeCoord + normal.z * (radius * 1.18f + 0.035f);
                }
                float nearWall = Clamp01(1.0f - distanceToPlane / std::max(0.1f, maze_.TileMinimum() * 0.44f));
                float wallHunger = 0.22f + std::abs(surfaceNoiseC) * 0.42f + nearWall * 0.45f;
                float score = distanceToPlane / std::max(0.1f, maze_.TileMinimum()) - wallHunger;
                if (score < best.score) best = {normal, c, score};
            };
            considerWall(1, 0, {-1.0f, 0.0f, 0.0f}, tileCenter.x + halfW, halfW - localX);
            considerWall(-1, 0, {1.0f, 0.0f, 0.0f}, tileCenter.x - halfW, halfW + localX);
            considerWall(0, 1, {0.0f, 0.0f, -1.0f}, tileCenter.z + halfD, halfD - localZ);
            considerWall(0, -1, {0.0f, 0.0f, 1.0f}, tileCenter.z - halfD, halfD + localZ);
            XMFLOAT3 surfaceUp = Normalize3(best.normal, up);
            if (i > 0) {
                XMFLOAT3 prevUp = bodyUps[static_cast<size_t>(i - 1)];
                float continuity = 0.86f;
                surfaceUp = Normalize3(Lerp3(surfaceUp, prevUp, continuity), prevUp);
                if (Dot3(surfaceUp, prevUp) < 0.0f) surfaceUp = Scale3(surfaceUp, -1.0f);
                surfaceUp = Normalize3(Sub3(surfaceUp, Scale3(tangent, Dot3(surfaceUp, tangent))), prevUp);
            }
            p = best.center;
            if (i > 0) {
                XMFLOAT3 parent = bodyPoints[static_cast<size_t>(i - 1)];
                XMFLOAT3 trailDelta = Sub3(bodyCenterlinePoints[static_cast<size_t>(i)], bodyCenterlinePoints[static_cast<size_t>(i - 1)]);
                XMFLOAT3 parentTrajectory = Add3(parent, trailDelta);
                XMFLOAT3 surfaceOffset = Sub3(p, parentTrajectory);
                float drift = Length3(surfaceOffset);
                float maxSurfaceDrift = std::max(0.06f, bodySpacing * 0.38f);
                if (drift > maxSurfaceDrift) {
                    surfaceOffset = Scale3(surfaceOffset, maxSurfaceDrift / std::max(0.001f, drift));
                }
                p = Add3(parentTrajectory, surfaceOffset);
            }
            float gait = time_ * 3.65f - fi * 0.74f + monster_.x * 0.09f + monster_.z * 0.05f;
            float stepLift = std::pow(Clamp01(std::sin(gait) * 0.5f + 0.5f), 2.3f);
            float gutPulse = std::sin(time_ * 2.65f - fi * 0.58f) * 0.065f;
            float bodyBounce = std::sin(time_ * 5.35f - fi * 0.92f) * 0.038f * (0.55f + taper * 0.45f);
            p = Add3(p, Scale3(surfaceUp, stepLift * 0.070f + bodyBounce + gutPulse * 0.55f));
            bodyUps[static_cast<size_t>(i)] = surfaceUp;
            bodySides[static_cast<size_t>(i)] = Normalize3(Cross3(surfaceUp, tangent), side);
            bodyPoints[static_cast<size_t>(i)] = p;
        }
        auto constrainBodyChain = [&]() {
            float maxSegmentDistance = std::max(0.16f, bodySpacing * 1.18f);
            float minSegmentDistance = std::max(0.08f, bodySpacing * 0.62f);
            for (int pass = 0; pass < 3; ++pass) {
                for (int i = 1; i < bodyCount; ++i) {
                    XMFLOAT3 prev = bodyPoints[static_cast<size_t>(i - 1)];
                    XMFLOAT3 cur = bodyPoints[static_cast<size_t>(i)];
                    XMFLOAT3 delta = Sub3(cur, prev);
                    float dist = Length3(delta);
                    if (dist > maxSegmentDistance) {
                        XMFLOAT3 dir = Scale3(delta, 1.0f / std::max(0.001f, dist));
                        bodyPoints[static_cast<size_t>(i)] = Add3(prev, Scale3(dir, maxSegmentDistance));
                    } else if (dist < minSegmentDistance && dist > 0.001f) {
                        XMFLOAT3 dir = Scale3(delta, 1.0f / dist);
                        bodyPoints[static_cast<size_t>(i)] = Add3(prev, Scale3(dir, minSegmentDistance));
                    }
                }
                for (int i = bodyCount - 2; i >= 1; --i) {
                    XMFLOAT3 next = bodyPoints[static_cast<size_t>(i + 1)];
                    XMFLOAT3 cur = bodyPoints[static_cast<size_t>(i)];
                    XMFLOAT3 delta = Sub3(cur, next);
                    float dist = Length3(delta);
                    if (dist > maxSegmentDistance) {
                        XMFLOAT3 dir = Scale3(delta, 1.0f / std::max(0.001f, dist));
                        bodyPoints[static_cast<size_t>(i)] = Add3(next, Scale3(dir, maxSegmentDistance));
                    }
                }
            }
        };
        constrainBodyChain();

        float smoothDt = std::clamp(time_ - monsterBodySmoothTime_, 0.0f, 0.05f);
        bool resetBodySmoothing = monsterSmoothedBodyPoints_.size() != static_cast<size_t>(bodyCount) ||
            monsterSmoothedBodyUps_.size() != static_cast<size_t>(bodyCount) ||
            monsterBodySmoothTime_ < -100.0f ||
            smoothDt <= 0.0001f;
        if (resetBodySmoothing) {
            monsterSmoothedBodyPoints_.assign(bodyPoints.begin(), bodyPoints.begin() + bodyCount);
            monsterSmoothedBodyUps_.assign(bodyUps.begin(), bodyUps.begin() + bodyCount);
        } else {
            for (int i = 0; i < bodyCount; ++i) {
                float follow = Lerp(14.0f, 6.0f, static_cast<float>(i) / std::max(1.0f, static_cast<float>(bodyCount - 1)));
                float alpha = 1.0f - std::exp(-smoothDt * follow);
                XMFLOAT3 target = bodyPoints[static_cast<size_t>(i)];
                XMFLOAT3 prev = monsterSmoothedBodyPoints_[static_cast<size_t>(i)];
                if (Length3(Sub3(target, prev)) > maze_.TileAverage() * 1.6f) {
                    monsterSmoothedBodyPoints_[static_cast<size_t>(i)] = target;
                } else {
                    monsterSmoothedBodyPoints_[static_cast<size_t>(i)] = Lerp3(prev, target, alpha);
                }
                XMFLOAT3 prevUp = monsterSmoothedBodyUps_[static_cast<size_t>(i)];
                XMFLOAT3 targetUp = bodyUps[static_cast<size_t>(i)];
                if (Dot3(prevUp, targetUp) < 0.0f) targetUp = Scale3(targetUp, -1.0f);
                monsterSmoothedBodyUps_[static_cast<size_t>(i)] = Normalize3(Lerp3(prevUp, targetUp, 1.0f - std::exp(-smoothDt * 4.8f)), targetUp);
            }
            std::copy(monsterSmoothedBodyPoints_.begin(), monsterSmoothedBodyPoints_.begin() + bodyCount, bodyPoints.begin());
            std::copy(monsterSmoothedBodyUps_.begin(), monsterSmoothedBodyUps_.begin() + bodyCount, bodyUps.begin());
            constrainBodyChain();
            std::copy(bodyPoints.begin(), bodyPoints.begin() + bodyCount, monsterSmoothedBodyPoints_.begin());
        }
        monsterBodySmoothTime_ = time_;

        for (int i = 0; i < bodyCount; ++i) {
            XMFLOAT3 prev = i > 0 ? bodyPoints[static_cast<size_t>(i - 1)] : bodyPoints[static_cast<size_t>(i)];
            XMFLOAT3 next = i + 1 < bodyCount ? bodyPoints[static_cast<size_t>(i + 1)] : bodyPoints[static_cast<size_t>(i)];
            XMFLOAT3 tangent = Normalize3(Sub3(prev, next), bodyTangents[static_cast<size_t>(i)]);
            bodyTangents[static_cast<size_t>(i)] = tangent;
            XMFLOAT3 upProjected = Normalize3(Sub3(bodyUps[static_cast<size_t>(i)], Scale3(tangent, Dot3(bodyUps[static_cast<size_t>(i)], tangent))),
                i > 0 ? bodyUps[static_cast<size_t>(i - 1)] : up);
            if (i > 0 && Dot3(upProjected, bodyUps[static_cast<size_t>(i - 1)]) < 0.0f) {
                upProjected = Scale3(upProjected, -1.0f);
            }
            bodyUps[static_cast<size_t>(i)] = upProjected;
            bodySides[static_cast<size_t>(i)] = Normalize3(Cross3(bodyUps[static_cast<size_t>(i)], tangent), bodySides[static_cast<size_t>(i)]);
        }
        constexpr int tubeSides = 20;
        struct TubeVertex {
            XMFLOAT3 pos;
            XMFLOAT3 normal;
            XMFLOAT3 tangent;
            XMFLOAT2 uv;
        };
        auto tubeVertex = [&](int idx, int ring) {
            float baseU = static_cast<float>(ring) / static_cast<float>(tubeSides);
            float u = std::fmod(baseU + bodyUvShift[static_cast<size_t>(idx)], 1.0f);
            float angle = u * kPi * 2.0f;
            float radius = bodyRadii[static_cast<size_t>(idx)];
            float idxF = static_cast<float>(idx);
            float surfaceLump = 1.0f + std::sin(angle * 3.0f + idxF * 0.61f + time_ * 1.1f) * 0.075f +
                std::sin(angle * 5.0f + idxF * 0.37f) * 0.040f;
            float ovalYScale = 0.72f + std::sin(time_ * 3.2f + idxF * 0.63f) * 0.040f;
            float ovalXScale = 1.06f + std::sin(time_ * 3.7f + idxF * 0.77f) * 0.055f;
            float ovalY = radius * surfaceLump * ovalYScale;
            float ovalX = radius * surfaceLump * ovalXScale;
            XMFLOAT3 side = bodySides[static_cast<size_t>(idx)];
            XMFLOAT3 tubeUp = bodyUps[static_cast<size_t>(idx)];
            XMFLOAT3 tangent = bodyTangents[static_cast<size_t>(idx)];
            XMFLOAT3 normal = Normalize3(Add3(Scale3(side, std::cos(angle) / std::max(0.001f, ovalXScale)),
                Scale3(tubeUp, std::sin(angle) / std::max(0.001f, ovalYScale))), tubeUp);
            XMFLOAT3 pos = Add3(bodyPoints[static_cast<size_t>(idx)],
                Add3(Scale3(side, std::cos(angle) * ovalX),
                     Scale3(tubeUp, std::sin(angle) * ovalY)));
            float v = bodyUvV[static_cast<size_t>(idx)] + std::sin(idxF * 1.31f + baseU * kPi * 2.0f) * 0.035f;
            return TubeVertex{pos, normal, tangent, {u, v}};
        };
        auto pushTubeTri = [&](const TubeVertex& a, const TubeVertex& b, const TubeVertex& c, float material) {
            solidVerts.push_back({a.pos, a.normal, a.tangent, a.uv, material});
            solidVerts.push_back({b.pos, b.normal, b.tangent, b.uv, material});
            solidVerts.push_back({c.pos, c.normal, c.tangent, c.uv, material});
        };
        for (int i = 0; i + 1 < bodyCount; ++i) {
            for (int r = 0; r < tubeSides; ++r) {
                TubeVertex a = tubeVertex(i, r);
                TubeVertex b = tubeVertex(i, r + 1);
                TubeVertex c = tubeVertex(i + 1, r + 1);
                TubeVertex d = tubeVertex(i + 1, r);
                pushTubeTri(a, b, c, gutMat);
                pushTubeTri(a, c, d, gutMat);
            }
        }
        AppendDynamicEllipsoid(solidVerts, bodyPoints[static_cast<size_t>(bodyCount - 1)],
            bodySides[static_cast<size_t>(bodyCount - 1)], bodyUps[static_cast<size_t>(bodyCount - 1)], bodyTangents[static_cast<size_t>(bodyCount - 1)],
            {bodyRadii[static_cast<size_t>(bodyCount - 1)] * 0.82f, bodyRadii[static_cast<size_t>(bodyCount - 1)] * 0.52f,
             bodyRadii[static_cast<size_t>(bodyCount - 1)] * 0.94f}, 12, 6, gutMat);

        int limbPairs = std::min(bodyCount - 3, std::max(8, bodyCount - 4));
        float limbStep = std::max(1.0f, static_cast<float>(bodyCount - 4) / std::max(1.0f, static_cast<float>(limbPairs)));
        int requiredLimbAnchors = std::max(0, limbPairs * 2);
        if (monsterLimbAnchors_.size() != static_cast<size_t>(requiredLimbAnchors)) {
            monsterLimbAnchors_.resize(static_cast<size_t>(requiredLimbAnchors));
        }
        struct LimbReachTarget {
            XMFLOAT3 target;
            XMFLOAT3 normal;
            float swing;
        };
        auto limbReachTarget = [&](XMFLOAT3 root, XMFLOAT3 sideDir, XMFLOAT3 tangentDir, XMFLOAT3 upDir, int limbId, float upperLen, float lowerLen) {
            MonsterLimbAnchor& anchor = monsterLimbAnchors_[static_cast<size_t>(limbId)];
            float maxReach = std::max(0.18f, upperLen + lowerLen);
            float reachVariance = Rand01(limbId * 17 + 3, 787, runtimeSeed_);
            float detachReach = maxReach * (0.74f + reachVariance * 0.22f);
            float plantSlack = maxReach * (0.18f + Rand01(limbId * 19 + 5, 797, runtimeSeed_) * 0.12f);
            float now = time_;
            if (!anchor.planted && anchor.swingDuration <= 0.001f) {
                anchor.swingDuration = 0.46f + Rand01(limbId * 23 + 7, 803, runtimeSeed_) * 0.42f;
            }
            if (!anchor.planted && Length3(Sub3(anchor.swingTo, anchor.swingFrom)) <= 0.001f) {
                SurfaceContact contact = contactPoint(root, sideDir, tangentDir, upDir, limbId, anchor.retargetCount);
                anchor.anchor = contact.point;
                anchor.anchorNormal = contact.normal;
                anchor.swingFrom = anchor.anchor;
                anchor.swingTo = anchor.anchor;
                anchor.swingFromNormal = anchor.anchorNormal;
                anchor.swingToNormal = anchor.anchorNormal;
                anchor.planted = true;
                recordHandprint(anchor.anchor, anchor.anchorNormal, limbId, anchor.retargetCount);
            }
            if (anchor.planted) {
                float stretch = Length3(Sub3(anchor.anchor, root));
                float impatient = std::sin(now * (0.31f + Rand01(limbId, 809, runtimeSeed_) * 0.19f) + Rand01(limbId, 811, runtimeSeed_) * kPi * 2.0f);
                bool wantsReplant = stretch > detachReach ||
                    (stretch > detachReach - plantSlack && impatient > 0.92f);
                if (wantsReplant) {
                    anchor.planted = false;
                    anchor.swingFrom = anchor.anchor;
                    anchor.swingFromNormal = anchor.anchorNormal;
                    anchor.retargetCount += 1;
                    SurfaceContact contact = contactPoint(root, sideDir, tangentDir, upDir, limbId, anchor.retargetCount);
                    anchor.swingTo = contact.point;
                    anchor.swingToNormal = contact.normal;
                    anchor.swingStart = now;
                    anchor.swingDuration = 0.38f + Rand01(limbId + anchor.retargetCount * 13, 811, runtimeSeed_) * 0.54f;
                } else {
                    return LimbReachTarget{anchor.anchor, anchor.anchorNormal, 0.0f};
                }
            }
            float swingT = Clamp01((now - anchor.swingStart) / std::max(0.001f, anchor.swingDuration));
            float eased = SmoothStep(0.0f, 1.0f, swingT);
            eased = eased * eased * eased * (eased * (eased * 6.0f - 15.0f) + 10.0f);
            if (swingT >= 0.999f) {
                anchor.anchor = anchor.swingTo;
                anchor.anchorNormal = anchor.swingToNormal;
                anchor.planted = true;
                recordHandprint(anchor.anchor, anchor.anchorNormal, limbId, anchor.retargetCount);
                return LimbReachTarget{anchor.anchor, anchor.anchorNormal, 0.0f};
            }
            float arc = std::sin(eased * kPi) * (0.16f + Rand01(limbId, anchor.retargetCount + 853, runtimeSeed_) * 0.10f);
            XMFLOAT3 arcDir = up;
            if (anchor.swingTo.y > settings_.wallHeightMeters - 0.12f) {
                arcDir = Scale3(up, -1.0f);
            } else if (anchor.swingTo.y > 0.10f) {
                arcDir = Normalize3(Add3(up, Scale3(sideDir, 0.38f)), up);
            }
            XMFLOAT3 target = Add3(Lerp3(anchor.swingFrom, anchor.swingTo, eased), Scale3(arcDir, arc));
            XMFLOAT3 normal = Normalize3(Lerp3(anchor.swingFromNormal, anchor.swingToNormal, eased), up);
            return LimbReachTarget{target, normal, 1.0f - std::abs(eased * 2.0f - 1.0f)};
        };
        for (int pairIndex = 0; pairIndex < limbPairs; ++pairIndex) {
            int i = std::clamp(2 + static_cast<int>(std::round(static_cast<float>(pairIndex) * limbStep)), 2, bodyCount - 2);
            XMFLOAT3 p = bodyPoints[static_cast<size_t>(i)];
            XMFLOAT3 prev = bodyPoints[static_cast<size_t>(std::max(0, i - 1))];
            XMFLOAT3 next = bodyPoints[static_cast<size_t>(std::min(bodyCount - 1, i + 1))];
            XMFLOAT3 tangent = Normalize3(Sub3(prev, next), forward);
            XMFLOAT3 localUp = bodyUps[static_cast<size_t>(i)];
            XMFLOAT3 sideAxis = Normalize3(Cross3(localUp, tangent), right);
            for (int sideIndex = -1; sideIndex <= 1; sideIndex += 2) {
                int limbId = pairIndex * 2 + (sideIndex > 0 ? 1 : 0);
                float limbSeed = Rand01(limbId * 29 + 1, 823, runtimeSeed_);
                float limbPhase = limbSeed * kPi * 2.0f;
                float phase = time_ * (1.20f + limbSeed * 0.92f + monsterHeadChaseBlend_ * (0.75f + limbSeed * 0.85f)) +
                    static_cast<float>(i) * (0.27f + limbSeed * 0.31f) + static_cast<float>(sideIndex) * (1.10f + limbSeed) + limbPhase;
                float sideScale = static_cast<float>(sideIndex);
                XMFLOAT3 sideDir = Scale3(sideAxis, sideScale);
                float r = bodyRadii[static_cast<size_t>(i)];
                float lateralRoot = r * (0.80f + limbSeed * 0.34f);
                float axialRoot = (limbSeed - 0.5f) * bodySpacing * 0.38f;
                XMFLOAT3 root = Add3(p, Add3(Scale3(sideDir, lateralRoot),
                    Add3(Scale3(tangent, axialRoot), Scale3(localUp, std::sin(phase) * (0.025f + limbSeed * 0.040f)))));
                float limb = Lerp(0.034f, 0.019f, static_cast<float>(i) / static_cast<float>(bodyCount)) * (0.88f + limbSeed * 0.28f);
                float upperLen = std::max(0.14f, maze_.TileMinimum() * (0.23f + limbSeed * 0.075f) + limb * 1.28f);
                float lowerLen = std::max(0.14f, maze_.TileMinimum() * (0.27f + Rand01(limbId * 31 + 9, 829, runtimeSeed_) * 0.085f) + limb * 1.04f);
                LimbReachTarget reachTarget = limbReachTarget(root, sideDir, tangent, localUp, limbId, upperLen, lowerLen);
                float swing = reachTarget.swing;
                XMFLOAT3 target = reachTarget.target;
                XMFLOAT3 contactNormal = Normalize3(reachTarget.normal, up);
                bool chaseReachArm = canTrackPlayer && monsterHeadChaseBlend_ > 0.34f && pairIndex < 2;
                if (chaseReachArm) {
                    float reachWeight = SmoothStep(0.34f, 0.92f, monsterHeadChaseBlend_) * (pairIndex == 0 ? 1.0f : 0.76f);
                    XMFLOAT3 playerAim{camera_.x, camera_.y - 0.10f + static_cast<float>(pairIndex) * 0.16f, camera_.z};
                    XMFLOAT3 toPlayer = Sub3(playerAim, root);
                    float playerDist = Length3(toPlayer);
                    XMFLOAT3 playerDir = Normalize3(toPlayer, forward);
                    float reachLen = std::min(playerDist, (upperLen + lowerLen) * (0.92f + 0.06f * std::sin(time_ * 5.2f + limbId)));
                    XMFLOAT3 reachPoint = Add3(root, Scale3(playerDir, reachLen));
                    reachPoint = Add3(reachPoint, Add3(Scale3(sideDir, std::sin(time_ * 6.8f + limbId * 1.7f) * 0.045f),
                        Scale3(localUp, std::sin(time_ * 9.1f + limbId * 0.9f) * 0.035f)));
                    target = Lerp3(target, reachPoint, reachWeight);
                    contactNormal = Scale3(playerDir, -1.0f);
                    swing = std::max(swing, 0.32f + reachWeight * 0.48f);
                }
                XMFLOAT3 toTarget = Sub3(target, root);
                float reach = Length3(toTarget);
                XMFLOAT3 reachDir = Normalize3(toTarget, sideDir);
                float clampedReach = std::clamp(reach, 0.05f, upperLen + lowerLen - 0.025f);
                float along = (upperLen * upperLen - lowerLen * lowerLen + clampedReach * clampedReach) / std::max(0.001f, 2.0f * clampedReach);
                float bendHeight = std::sqrt(std::max(0.0f, upperLen * upperLen - along * along));
                XMFLOAT3 bendAxis = Normalize3(Add3(Scale3(up, 0.82f), Add3(Scale3(sideDir, 0.34f), Scale3(tangent, std::sin(phase) * 0.30f))), up);
                if (target.y < 0.08f) {
                    bendAxis = Normalize3(Add3(Scale3(sideDir, 0.72f), Add3(Scale3(up, 0.42f), Scale3(tangent, std::sin(phase) * 0.22f))), sideDir);
                } else if (target.y > settings_.wallHeightMeters - 0.12f) {
                    bendAxis = Normalize3(Add3(Scale3(sideDir, 0.65f), Add3(Scale3(up, -0.48f), Scale3(tangent, std::sin(phase) * 0.22f))), sideDir);
                }
                XMFLOAT3 knee = Add3(root, Add3(Scale3(reachDir, along), Scale3(bendAxis, bendHeight * (0.76f + swing * 0.34f))));
                knee = Add3(knee, Add3(Scale3(tangent, std::sin(phase * 0.67f) * 0.10f), Scale3(up, swing * 0.11f)));
                organicSegment(root, knee, limb * 3.25f, limb * 1.34f, limbMat + std::fmod(static_cast<float>(limbId) * 0.011f, 0.16f), 10);
                organicSegment(knee, target, limb * 1.52f, limb * 0.72f, limbMat + std::fmod(static_cast<float>(limbId) * 0.013f, 0.16f), 9);
                AppendDynamicEllipsoid(solidVerts, root, sideAxis, localUp, tangent,
                    {limb * 3.05f, limb * 2.20f, limb * 3.20f}, 10, 6, limbMat + 0.08f);
                AppendDynamicEllipsoid(solidVerts, knee, sideAxis, localUp, tangent,
                    {limb * 1.92f, limb * 1.58f, limb * 1.92f}, 8, 5, limbMat + 0.10f);
                XMFLOAT3 palmRight{};
                XMFLOAT3 palmUp{};
                surfaceAxes(contactNormal, palmRight, palmUp);
                XMFLOAT3 palmCenter = Add3(target, Scale3(contactNormal, 0.016f + swing * 0.014f));
                AppendDynamicEllipsoid(solidVerts, palmCenter, palmRight, palmUp, contactNormal,
                    {limb * 2.35f, limb * 1.55f, limb * 0.82f}, 10, 6, limbMat + 0.36f);
                for (int finger = -2; finger <= 2; ++finger) {
                    float f = static_cast<float>(finger);
                    float spread = f * (0.040f + limb * 0.30f) * (1.0f + swing * 0.45f);
                    float reachOut = (0.105f + (2.0f - std::abs(f)) * 0.014f) * (1.0f - swing * 0.18f);
                    XMFLOAT3 base = Add3(palmCenter, Add3(Scale3(palmRight, spread), Scale3(palmUp, limb * 1.05f)));
                    XMFLOAT3 tip = Add3(base, Add3(Scale3(palmUp, reachOut), Scale3(palmRight, f * 0.018f * (1.0f + swing))));
                    tip = Add3(tip, Scale3(contactNormal, 0.010f + swing * 0.040f));
                    organicSegment(base, tip, limb * 0.45f, limb * 0.22f, limbMat + 0.18f, 7);
                }
                XMFLOAT3 thumbBase = Add3(palmCenter, Add3(Scale3(palmRight, limb * 1.95f * static_cast<float>(sideIndex)), Scale3(palmUp, limb * 0.10f)));
                XMFLOAT3 thumbTip = Add3(thumbBase, Add3(Scale3(palmRight, limb * 2.15f * static_cast<float>(sideIndex)), Scale3(palmUp, limb * 2.15f)));
                thumbTip = Add3(thumbTip, Scale3(contactNormal, 0.012f + swing * 0.035f));
                organicSegment(thumbBase, thumbTip, limb * 0.50f, limb * 0.24f, limbMat + 0.18f, 7);
            }
        }

        constexpr int monsterSmokePuffs = 0;
        for (int i = 0; i < monsterSmokePuffs; ++i) {
            float fi = static_cast<float>(i);
            float a = fi * 2.399963f + std::sin(time_ * (0.31f + fi * 0.017f)) * 0.34f;
            float layer = std::fmod(fi * 0.618034f, 1.0f);
            float column = std::fmod(fi * 0.381966f, 1.0f) - 0.5f;
            float lower = 1.0f - layer;
            float y = 0.05f + layer * 1.64f + std::sin(time_ * (0.74f + fi * 0.023f) + fi) * (0.060f + lower * 0.048f);
            float radius = 0.06f + 0.30f * (1.0f - std::abs(layer - 0.52f)) + lower * 0.28f;
            float x = std::cos(a) * radius + std::sin(time_ * 0.47f + fi) * 0.045f;
            float z = std::sin(a) * radius + kMonsterSmokeBackOffset * 0.34f + column * 0.20f +
                std::cos(time_ * 0.39f + fi * 1.7f) * 0.045f;
            float pulse = 0.88f + std::sin(time_ * (0.68f + fi * 0.031f) + fi * 3.1f) * 0.12f;
            float lowerScale = Lerp(1.04f, 2.55f, std::pow(lower, 1.30f));
            smokePuff(x, y, z,
                (0.166f + 0.104f * (1.0f - std::abs(layer - 0.50f))) * pulse * lowerScale,
                (0.156f + 0.122f * (1.0f - std::abs(layer - 0.45f))) * pulse * lowerScale,
                smokeMaterial(1.07f + fi * 0.043f));
        }

        const int capeStrips = 0;
        for (int i = 0; i < capeStrips; ++i) {
            float a0 = (static_cast<float>(i) / static_cast<float>(capeStrips)) * kPi * 2.0f;
            float a1 = (static_cast<float>(i + 1) / static_cast<float>(capeStrips)) * kPi * 2.0f;
            float wave0 = std::sin(time_ * 2.6f + a0 * 2.0f + monster_.x * 0.11f) * 0.035f;
            float wave1 = std::sin(time_ * 2.6f + a1 * 2.0f + monster_.z * 0.09f) * 0.035f;
            float topR0 = 0.33f + wave0 * 0.60f;
            float topR1 = 0.33f + wave1 * 0.60f;
            float midR0 = 0.50f + wave0 * 1.35f;
            float midR1 = 0.50f + wave1 * 1.35f;
            float botR0 = 0.40f + wave0 * 1.15f;
            float botR1 = 0.40f + wave1 * 1.15f;
            float torn0 = std::sin(static_cast<float>(i) * 2.91f + monster_.x) * 0.055f;
            float torn1 = std::sin(static_cast<float>(i + 1) * 2.91f + monster_.z) * 0.055f;

            XMFLOAT3 pTop0 = off(std::cos(a0) * topR0, 1.52f + breathe * 0.45f, std::sin(a0) * topR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pTop1 = off(std::cos(a1) * topR1, 1.52f + breathe * 0.45f, std::sin(a1) * topR1 + kMonsterSmokeBackOffset);
            XMFLOAT3 pMid0 = off(std::cos(a0) * midR0, 0.86f + wave0 * 0.55f + breathe, std::sin(a0) * midR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pMid1 = off(std::cos(a1) * midR1, 0.86f + wave1 * 0.55f + breathe, std::sin(a1) * midR1 + kMonsterSmokeBackOffset);
            XMFLOAT3 pBot0 = off(std::cos(a0) * botR0, 0.20f + torn0, std::sin(a0) * botR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pBot1 = off(std::cos(a1) * botR1, 0.20f + torn1, std::sin(a1) * botR1 + kMonsterSmokeBackOffset);
            float amid = (a0 + a1) * 0.5f;
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, std::cos(amid)), Scale3(forward, std::sin(amid))), forward);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, -std::sin(amid)), Scale3(forward, std::cos(amid))), right);
            float material = smokeMaterial(static_cast<float>(i) * 0.037f + monster_.x * 0.021f + monster_.z * 0.013f);
            AppendDynamicQuad(transparentVerts, pBot0, pBot1, pMid1, pMid0, normal, tangent, material);
            AppendDynamicQuad(transparentVerts, pMid0, pMid1, pTop1, pTop0, normal, tangent, material + 0.017f);
        }
        for (int i = 0; i < 0; ++i) {
            float a = static_cast<float>(i) / 7.0f * kPi * 2.0f + std::sin(time_ * 0.43f + i) * 0.22f;
            float wobble = std::sin(time_ * 1.1f + i * 2.3f) * 0.06f;
            XMFLOAT3 a0 = off(std::cos(a) * (0.10f + wobble), 1.42f + breathe, std::sin(a) * (0.10f + wobble) + kMonsterSmokeBackOffset);
            XMFLOAT3 a1 = off(std::cos(a + 0.38f) * (0.56f + wobble), 0.56f, std::sin(a + 0.38f) * (0.56f + wobble) + kMonsterSmokeBackOffset);
            XMFLOAT3 a2 = off(std::cos(a + 0.74f) * (0.86f + wobble), 0.03f, std::sin(a + 0.74f) * (0.86f + wobble) + kMonsterSmokeBackOffset);
            smokeBand(a0, a1, 0.09f, 0.20f, smokeMaterial(0.41f + i * 0.061f));
            smokeBand(a1, a2, 0.14f, 0.27f, smokeMaterial(0.63f + i * 0.053f));
        }
        // Keep the body as overlapping volumetric puffs; hard bands read as cards in preview.

        bool visualPlayerLock = canTrackPlayer && MonsterLineOfSightToPlayer();
        float liveLock = canTrackPlayer ? monsterHeadLockAmount_ : 0.0f;
        float headLock = std::max(deathHeadLock, liveLock);
        float scanWeight = 1.0f - Clamp01(headLock);
        float headYaw = faceYaw + monsterHeadYawOffset_ * scanWeight +
            twitch * (1.0f - deathHeadLock * 0.85f) * scanWeight;
        XMFLOAT3 hRight{std::cos(headYaw), 0.0f, -std::sin(headYaw)};
        XMFLOAT3 hUp = bodyUps[0];
        XMFLOAT3 hForward{std::sin(headYaw), 0.0f, std::cos(headYaw)};
        float upBlend = visualPlayerLock ? Lerp(0.22f, 0.82f, SmoothStep(0.0f, 1.0f, headLock)) : 0.22f;
        hUp = Normalize3(Lerp3(hUp, {0.0f, 1.0f, 0.0f}, upBlend), {0.0f, 1.0f, 0.0f});
        hRight = Normalize3(Cross3(hUp, hForward), hRight);
        hUp = Normalize3(Cross3(hForward, hRight), hUp);
        XMFLOAT3 skull = Add3(bodyPoints[0], OrientedOffset(hRight, hUp, hForward,
            0.0f, bodyRadii[0] * 0.70f + MonsterHeadBobOffset() * 0.55f * modelY, kMonsterHeadForwardOffset * 0.56f * modelXZ));
        float headPitch = monsterHeadPitchOffset_ * scanWeight;
        if (std::abs(headPitch) > 0.0005f) {
            hForward = Normalize3(Add3(Scale3(hForward, std::cos(headPitch)), Scale3(hUp, std::sin(headPitch))), hForward);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        if (headLock > 0.001f) {
            XMFLOAT3 cameraFocus{camera_.x, camera_.y + 0.04f, camera_.z};
            XMFLOAT3 lookForward = Normalize3(Sub3(cameraFocus, skull), hForward);
            float trackBlend = SmoothStep(0.0f, 1.0f, Clamp01(headLock));
            hForward = Normalize3(Lerp3(hForward, lookForward, trackBlend * (visualPlayerLock ? 0.88f : 0.62f)), lookForward);
            if (visualPlayerLock) hUp = Normalize3(Lerp3(hUp, {0.0f, 1.0f, 0.0f}, trackBlend * 0.42f), {0.0f, 1.0f, 0.0f});
            hRight = Normalize3(Cross3(hUp, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        if (canTrackPlayer && headLock > 0.62f && visualPlayerLock) {
            XMFLOAT3 cameraFocus{camera_.x, camera_.y + 0.04f, camera_.z};
            XMFLOAT3 toPlayer = Normalize3(Sub3(cameraFocus, skull), hForward);
            float centered = SmoothStep(std::cos(5.0f * kPi / 180.0f), std::cos(1.2f * kPi / 180.0f), Dot3(hForward, toPlayer));
            float rage = centered * SmoothStep(0.62f, 0.96f, headLock);
            if (rage > 0.001f) {
                float yawJitter = (std::sin(time_ * 74.0f + monster_.x * 3.1f) * 0.030f +
                    std::sin(time_ * 121.0f + monster_.z * 1.7f) * 0.017f) * rage;
                float pitchJitter = (std::sin(time_ * 91.0f + monster_.z * 2.4f) * 0.025f +
                    std::sin(time_ * 147.0f + monster_.x * 1.6f) * 0.013f) * rage;
                float rollJitter = (std::sin(time_ * 109.0f + monster_.x * 0.9f) * 0.050f +
                    std::sin(time_ * 173.0f + monster_.z * 0.8f) * 0.022f) * rage;

                hForward = Normalize3(Add3(Scale3(hForward, std::cos(yawJitter)), Scale3(hRight, std::sin(yawJitter))), hForward);
                hRight = Normalize3(Cross3(hUp, hForward), hRight);
                hForward = Normalize3(Add3(Scale3(hForward, std::cos(pitchJitter)), Scale3(hUp, std::sin(pitchJitter))), hForward);
                hUp = Normalize3(Cross3(hForward, hRight), hUp);
                hRight = Normalize3(Add3(Scale3(hRight, std::cos(rollJitter)), Scale3(hUp, std::sin(rollJitter))), hRight);
                hUp = Normalize3(Cross3(hForward, hRight), hUp);
                skull = Add3(skull, Add3(Scale3(hRight, std::sin(time_ * 137.0f) * 0.010f * rage),
                    Scale3(hUp, std::sin(time_ * 151.0f + 1.7f) * 0.008f * rage)));
            }
        }
        bool externalSkull = !skullMesh_.empty();
        XMFLOAT3 externalMaskCenter = skull;
        if (externalSkull) {
            XMFLOAT3 maskCenter = Add3(skull, Scale3(hForward, 0.145f * modelXZ));
            externalMaskCenter = maskCenter;
            XMFLOAT3 maskBackPlane = Add3(maskCenter, Scale3(hForward, -0.060f * 1.05f * modelXZ));
            XMFLOAT3 neckSocket = Add3(skull, Scale3(hForward, -0.060f * modelXZ));
            organicSegment(bodyPoints[0], neckSocket, bodyRadii[0] * 0.76f, 0.305f * modelXZ, gutMat + 0.035f, 14);
            organicSegment(neckSocket, maskBackPlane, 0.305f * modelXZ, 0.405f * modelXZ, gutMat + 0.045f, 18);
            AppendDynamicEllipsoid(solidVerts, neckSocket, hRight, hUp, hForward,
                {0.450f * modelXZ, 0.575f * modelY, 0.145f * modelXZ}, 22, 10, gutMat + 0.04f);
            AppendDynamicEllipsoid(solidVerts, Add3(maskBackPlane, Scale3(hForward, -0.018f * modelXZ)), hRight, hUp, hForward,
                {0.505f * modelXZ, 0.665f * modelY, 0.070f * modelXZ}, 26, 10, gutMat + 0.055f);
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Scale3(hForward, -0.360f * modelXZ)), hRight, hUp, hForward,
                {0.285f * modelXZ, 0.370f * modelY, 0.060f * modelXZ}, 16, 8, gutMat + 0.04f);
            AppendExternalSkullMesh(solidVerts, maskCenter, hRight, hUp, hForward,
                1.05f * modelXZ, 1.05f * modelY);
        } else {
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Scale3(hForward, -0.075f * modelXZ)), hRight, hUp, hForward,
                {0.410f * modelXZ, 0.515f * modelY, 0.185f * modelXZ}, 22, 12, gutMat + 0.04f);
            XMFLOAT3 maskCenter = Add3(skull, Scale3(hForward, 0.118f * modelXZ));
            AppendDynamicEllipsoid(solidVerts, maskCenter, hRight, hUp, hForward,
                {0.470f * modelXZ, 0.665f * modelY, 0.060f * modelXZ}, 28, 18, boneMat);
            AppendDynamicEllipsoid(solidVerts, Add3(maskCenter, Add3(Scale3(hForward, 0.076f * modelXZ), Scale3(hUp, 0.168f * modelY))), hRight, hUp, hForward,
                {0.405f * modelXZ, 0.050f * modelY, 0.023f * modelXZ}, 18, 5, darkMat);
            AppendDynamicEllipsoid(solidVerts, Add3(maskCenter, Add3(Scale3(hForward, 0.079f * modelXZ), Scale3(hUp, -0.095f * modelY))), hRight, hUp, hForward,
                {0.365f * modelXZ, 0.048f * modelY, 0.026f * modelXZ}, 22, 5, darkMat);
            AppendDynamicEllipsoid(solidVerts, Add3(maskCenter, Add3(Scale3(hForward, 0.045f * modelXZ), Scale3(hUp, -0.260f * modelY))), hRight, hUp, hForward,
                {0.132f * modelXZ, 0.175f * modelY, 0.030f * modelXZ}, 14, 8, darkMat);
            for (int t = -5; t <= 5; ++t) {
                float tx = static_cast<float>(t) * 0.064f;
                XMFLOAT3 tooth = Add3(maskCenter, OrientedOffset(hRight, hUp, hForward,
                    tx * modelXZ, (-0.120f + std::abs(static_cast<float>(t)) * 0.009f) * modelY, 0.102f * modelXZ));
                AppendDynamicEllipsoid(solidVerts, tooth, hRight, hUp, hForward,
                    {0.010f * modelXZ, (0.050f + std::abs(static_cast<float>(t)) * 0.004f) * modelY, 0.010f * modelXZ}, 8, 5, 9.98f);
            }
            for (int side = -1; side <= 1; side += 2) {
                XMFLOAT3 cheekA = Add3(maskCenter, OrientedOffset(hRight, hUp, hForward, 0.145f * side * modelXZ, -0.050f * modelY, 0.070f * modelXZ));
                XMFLOAT3 cheekB = Add3(maskCenter, OrientedOffset(hRight, hUp, hForward, 0.280f * side * modelXZ, -0.158f * modelY, 0.078f * modelXZ));
                organicSegment(cheekA, cheekB, 0.018f * modelXZ, 0.012f * modelXZ, darkMat, 8);
                XMFLOAT3 browA = Add3(maskCenter, OrientedOffset(hRight, hUp, hForward, 0.052f * side * modelXZ, 0.196f * modelY, 0.082f * modelXZ));
                XMFLOAT3 browB = Add3(maskCenter, OrientedOffset(hRight, hUp, hForward, 0.235f * side * modelXZ, 0.132f * modelY, 0.090f * modelXZ));
                organicSegment(browA, browB, 0.024f * modelXZ, 0.013f * modelXZ, darkMat, 8);
                XMFLOAT3 tearA = Add3(maskCenter, OrientedOffset(hRight, hUp, hForward, 0.172f * side * modelXZ, 0.062f * modelY, 0.092f * modelXZ));
                XMFLOAT3 tearB = Add3(maskCenter, OrientedOffset(hRight, hUp, hForward, (0.138f + 0.020f * side) * side * modelXZ, -0.235f * modelY, 0.100f * modelXZ));
                organicSegment(tearA, tearB, 0.010f * modelXZ, 0.006f * modelXZ, darkMat, 7);
            }
        }

        if (false && !externalSkull) {
            auto hornPoint = [&](float x, float y, float z) {
                return Add3(skull, OrientedOffset(hRight, hUp, hForward, x * modelXZ, y * modelY, z * modelXZ));
            };
            for (int side = -1; side <= 1; side += 2) {
                XMFLOAT3 h0 = hornPoint(0.10f * side, 0.11f, -0.035f);
                XMFLOAT3 h1 = hornPoint(0.30f * side, 0.38f, -0.080f);
                XMFLOAT3 h2 = hornPoint(0.56f * side, 0.55f, -0.120f);
                XMFLOAT3 h3 = hornPoint(0.75f * side, 0.61f, -0.080f);
                seg(h0, h1, 0.028f, 0.023f, boneMat);
                seg(h1, h2, 0.024f, 0.020f, boneMat);
                seg(h2, h3, 0.018f, 0.016f, boneMat);
                seg(h1, hornPoint(0.24f * side, 0.58f, 0.040f), 0.016f, 0.014f, boneMat);
                seg(h2, hornPoint(0.54f * side, 0.76f, -0.020f), 0.014f, 0.012f, boneMat);
                seg(h2, hornPoint(0.68f * side, 0.46f, 0.060f), 0.014f, 0.012f, boneMat);
            }
        }

        auto appendEye = [&](float xOffset, float eyeUp, float eyeForward, float variant) {
            float eyeHalfW = externalSkull ? 0.034f : 0.092f;
            float eyeHalfH = externalSkull ? 0.026f : 0.068f;
            XMFLOAT3 eyeBase = externalSkull ? externalMaskCenter : skull;
            XMFLOAT3 center = Add3(eyeBase, OrientedOffset(hRight, hUp, hForward,
                xOffset * modelXZ, eyeUp * modelY, eyeForward * modelXZ));
            XMFLOAT3 eyeNormal = hForward;
            XMFLOAT3 eyeRight = hRight;
            XMFLOAT3 eyeUpAxis = hUp;
            if (!externalSkull) {
                AppendDynamicEllipsoid(solidVerts, center, eyeRight, eyeUpAxis, eyeNormal,
                    {eyeHalfW * 0.92f * modelXZ, eyeHalfH * 0.92f * modelY, 0.028f * modelXZ}, 12, 7, darkMat);
            }
            if (externalSkull) {
                eyeNormal = hForward;
                eyeRight = hRight;
                eyeUpAxis = hUp;
                XMFLOAT3 orbCenter = Add3(center, Scale3(eyeNormal, 0.078f * modelXZ));
                AppendDynamicEllipsoid(solidVerts, orbCenter, eyeRight, eyeUpAxis, eyeNormal,
                    {0.038f * modelXZ, 0.030f * modelY, 0.018f * modelXZ}, 14, 8, 10.55f + variant * 0.025f);
            }
            XMFLOAT3 lensCenter = externalSkull ? Add3(center, Scale3(eyeNormal, 0.096f * modelXZ)) : center;
            XMFLOAT3 ew = Scale3(eyeRight, eyeHalfW * modelXZ);
            XMFLOAT3 eh = Scale3(eyeUpAxis, eyeHalfH * modelY);
            float eyeMaterial = externalSkull ? (10.58f + variant * 0.025f) : (12.05f + variant);
            AppendDynamicQuad(transparentVerts,
                Add3(lensCenter, Add3(Scale3(ew, -1.0f), Scale3(eh, -1.0f))),
                Add3(lensCenter, Add3(ew, Scale3(eh, -1.0f))),
                Add3(lensCenter, Add3(ew, eh)),
                Add3(lensCenter, Add3(Scale3(ew, -1.0f), eh)),
                eyeNormal, eyeRight, eyeMaterial);
        };
        if (externalSkull) {
            appendEye(-0.038f, 0.112f, 0.060f, 0.22f);
            appendEye(0.038f, 0.112f, 0.060f, 0.38f);
        } else {
            appendEye(-0.170f, 0.118f, 0.224f, 0.14f);
            appendEye(0.170f, 0.118f, 0.224f, 0.34f);
        }
    }

    void AppendSparkBillboards(std::vector<Vertex>& verts) {
        XMVECTOR cam = XMLoadFloat3(&camera_);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        for (const SparkParticle& spark : sparks_) {
            float lifeLeft = Clamp01(1.0f - spark.age / std::max(0.001f, spark.life));
            if (lifeLeft <= 0.0f) continue;
            XMVECTOR pos = XMLoadFloat3(&spark.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = spark.size * (0.55f + lifeLeft * lifeLeft * 1.35f);
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * size;
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 13.05f + lifeLeft * 0.38f);
        }
    }

    void AppendAirParticleBillboards(std::vector<Vertex>& verts) {
        if (!settings_.airParticles || airParticles_.empty() || monsterPreview_) return;
        XMFLOAT3 lightOrigin = FlashlightOrigin();
        XMFLOAT3 lightDir = Normalize3(FlashlightForward(), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 cameraForward = Normalize3(DirectionFromYawPitch(yaw_, lookPitch_), {0.0f, 0.0f, 1.0f});
        float maxDist = std::clamp(settings_.flashlightShadowDistanceMeters * 0.70f, 5.5f, 12.0f);
        float coneHalf = std::clamp(settings_.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneOuter = std::cos(coneHalf);
        float coneInner = std::cos(std::max(3.0f * kPi / 180.0f, coneHalf * 0.50f));
        int maxParticles = std::min<int>(static_cast<int>(airParticles_.size()), std::clamp(static_cast<int>(3400.0f * std::clamp(settings_.airParticleDensity, 0.0f, 4.0f)), 0, 11000));
        int emitted = 0;
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        for (const AirParticle& p : airParticles_) {
            if (emitted >= maxParticles) break;
            XMFLOAT3 pos = p.pos;
            XMFLOAT3 fromLight = Sub3(pos, lightOrigin);
            float lightDist = Length3(fromLight);
            if (lightDist < 0.30f || lightDist > maxDist) continue;
            XMFLOAT3 ray = Scale3(fromLight, 1.0f / std::max(0.001f, lightDist));
            float cone = SmoothStep(coneOuter, coneInner, Dot3(ray, lightDir));
            if (cone <= 0.018f) continue;
            XMFLOAT3 fromCamera = Sub3(pos, camera_);
            float cameraDepth = Dot3(fromCamera, cameraForward);
            if (cameraDepth <= 0.035f) continue;
            float focusBlur = Clamp01(Clamp01(std::abs(lightDist - airFocusDistance_) / (0.62f + lightDist * 0.18f)) *
                std::clamp(settings_.airParticleBlur, 0.0f, 3.0f));
            float distanceT = Clamp01(lightDist / maxDist);
            float distanceScale = Lerp(0.52f, 0.085f, distanceT);
            if (p.nearLayer > 0.5f) {
                distanceScale = std::max(distanceScale, p.nearLayer > 1.5f ? 0.92f : 0.72f);
            }
            float size = p.size * distanceScale * (1.0f + focusBlur * 0.24f);
            float projectedPixels = (size / std::max(0.06f, cameraDepth)) * static_cast<float>(std::max<LONG>(1, height_)) * 0.72f;
            if (p.nearLayer < 0.5f && projectedPixels < 0.22f) continue;
            float lifeFade = SmoothStep(0.0f, 2.8f, p.age) * (1.0f - SmoothStep(p.life - 5.2f, p.life, p.age));
            if (lifeFade <= 0.01f) continue;
            size *= Lerp(0.18f, 1.0f, lifeFade);
            XMFLOAT3 toCam = Normalize3(Sub3(camera_, pos), Scale3(lightDir, -1.0f));
            XMFLOAT3 right = Normalize3(Cross3(worldUp, toCam), {1.0f, 0.0f, 0.0f});
            XMFLOAT3 up = Normalize3(Cross3(toCam, right), worldUp);
            float angle = p.angle;
            XMFLOAT3 side = Normalize3(Add3(Scale3(right, std::cos(angle)), Scale3(up, std::sin(angle))), right);
            XMFLOAT3 vertical = Normalize3(Add3(Scale3(up, std::cos(angle)), Scale3(right, -std::sin(angle))), up);
            float aspect = std::clamp(p.aspect, 0.32f, 3.40f);
            float aspectRoot = std::sqrt(aspect);
            float halfW = size * std::clamp(aspectRoot, 0.58f, 1.84f);
            float halfH = size * std::clamp(1.0f / aspectRoot, 0.54f, 1.76f);
            XMFLOAT3 hw = Scale3(side, halfW);
            XMFLOAT3 hh = Scale3(vertical, halfH);
            float material = 15.0f + std::min(0.985f, lifeFade * 0.94f + p.seed * 0.035f);
            AppendDynamicQuad(verts,
                Add3(pos, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(pos, Add3(hw, Scale3(hh, -1.0f))),
                Add3(pos, Add3(hw, hh)),
                Add3(pos, Add3(Scale3(hw, -1.0f), hh)),
                toCam, side, material);
            ++emitted;
        }
    }

    void AppendSteamBillboards(std::vector<Vertex>& verts) {
        XMVECTOR cam = XMLoadFloat3(&camera_);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        for (const SteamParticle& sp : steam_) {
            float lifeLeft = Clamp01(1.0f - sp.age / std::max(0.001f, sp.life));
            if (lifeLeft <= 0.0f) continue;
            XMVECTOR pos = XMLoadFloat3(&sp.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = sp.size * (0.65f + (1.0f - lifeLeft) * 1.55f);
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * (size * 0.78f);
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 12.58f + lifeLeft * 0.35f);
        }
    }

    void AppendVentDrops(std::vector<Vertex>& verts) {
        for (const VentDrop& d : ventDrops_) {
            XMFLOAT3 right = RotateYVec({1.0f, 0.0f, 0.0f}, d.yaw);
            XMFLOAT3 forward = RotateYVec({0.0f, 0.0f, 1.0f}, d.yaw);
            XMVECTOR upVec = XMVector3Normalize(XMVectorSet(0, std::cos(d.roll), 0, 0) + XMLoadFloat3(&forward) * std::sin(d.roll));
            XMVECTOR rightVec = XMLoadFloat3(&right);
            XMVECTOR normalVec = XMVector3Normalize(XMVector3Cross(upVec, rightVec));
            XMVECTOR center = XMLoadFloat3(&d.pos);
            XMVECTOR halfW = rightVec * d.halfW;
            XMVECTOR halfH = upVec * d.halfH;
            XMFLOAT3 n, t, a, b, c, e;
            XMStoreFloat3(&n, normalVec);
            XMStoreFloat3(&t, rightVec);
            XMStoreFloat3(&a, center - halfW - halfH);
            XMStoreFloat3(&b, center + halfW - halfH);
            XMStoreFloat3(&c, center + halfW + halfH);
            XMStoreFloat3(&e, center - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, e, n, t, 10.0f);
            AppendDynamicQuad(verts, b, a, e, c, Scale3(n, -1.0f), Scale3(t, -1.0f), 10.0f);
        }
    }

    void UpdateDynamicGeometry() {
        std::vector<Vertex>& opaqueVerts = dynamicOpaqueVerts_;
        std::vector<Vertex>& transparentVerts = dynamicTransparentVerts_;
        opaqueVerts.clear();
        transparentVerts.clear();
        if (opaqueVerts.capacity() < 32768) opaqueVerts.reserve(32768);
        if (transparentVerts.capacity() < 131072) transparentVerts.reserve(131072);
        AppendDynamicDoor(opaqueVerts);
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            AppendMenuDoorwayLight(transparentVerts);
            AppendMenuButtonPlaques(opaqueVerts, transparentVerts);
        } else {
            AppendVentDrops(opaqueVerts);
            AppendMonsterBillboard(opaqueVerts, transparentVerts);
        }
        AppendAirParticleBillboards(transparentVerts);
        AppendSparkBillboards(transparentVerts);
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) AppendSteamBillboards(transparentVerts);
        if (opaqueVerts.size() > kDynamicVertexCapacity) opaqueVerts.resize(kDynamicVertexCapacity);
        size_t remaining = static_cast<size_t>(kDynamicVertexCapacity) - opaqueVerts.size();
        if (transparentVerts.size() > remaining) transparentVerts.resize(remaining);

        dynamicOpaqueVertexCount_ = static_cast<UINT>(opaqueVerts.size());
        dynamicTransparentVertexCount_ = static_cast<UINT>(transparentVerts.size());
        dynamicVertexCount_ = static_cast<UINT>(opaqueVerts.size() + transparentVerts.size());
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (dynamicBuffer_ && dynamicVertexCount_ > 0 &&
            SUCCEEDED(context_->Map(dynamicBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* dst = static_cast<uint8_t*>(mapped.pData);
            size_t opaqueBytes = opaqueVerts.size() * sizeof(Vertex);
            size_t transparentBytes = transparentVerts.size() * sizeof(Vertex);
            if (opaqueBytes > 0) {
                std::memcpy(dst, opaqueVerts.data(), opaqueBytes);
            }
            if (transparentBytes > 0) {
                std::memcpy(dst + opaqueBytes, transparentVerts.data(), transparentBytes);
            }
            context_->Unmap(dynamicBuffer_.Get(), 0);
        }
    }

    void UploadSceneConstants(const SceneConstants& cb) {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (SUCCEEDED(context_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            std::memcpy(mapped.pData, &cb, sizeof(cb));
            context_->Unmap(constantBuffer_.Get(), 0);
        }
    }

    void UploadLampDamageTexture() {
        if (!lampDamageDirty_ || !lampDamageTexture_ || lampDamagePixels_.empty()) return;
        context_->UpdateSubresource(
            lampDamageTexture_.Get(),
            0,
            nullptr,
            lampDamagePixels_.data(),
            static_cast<UINT>(maze_.w),
            0);
        lampDamageDirty_ = false;
    }

    std::array<XMFLOAT4, 2> ActiveSparkLights() const {
        std::array<XMFLOAT4, 2> lights{};
        std::array<float, 2> power{};
        for (const SparkFlash& flash : sparkFlashes_) {
            float lifeLeft = Clamp01(1.0f - flash.age / std::max(0.001f, flash.life));
            float p = flash.intensity * lifeLeft * lifeLeft;
            if (p <= 0.02f) continue;
            int slot = power[0] <= power[1] ? 0 : 1;
            if (p > power[slot]) {
                power[slot] = p;
                lights[static_cast<size_t>(slot)] = {flash.pos.x, flash.pos.y, flash.pos.z, p};
            }
        }
        return lights;
    }
