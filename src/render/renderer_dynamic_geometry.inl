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
            XMFLOAT3 local = RotateSkullLocalVector({-src.pos.x, src.pos.y, -src.pos.z});
            XMFLOAT3 nLocal = RotateSkullLocalVector({-src.normal.x, src.normal.y, -src.normal.z});
            XMFLOAT3 tLocal = RotateSkullLocalVector({-src.tangent.x, src.tangent.y, -src.tangent.z});
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
        float halfW = 0.55f;
        float halfH = 1.10f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        float angle = exitDoorAngle_;
        XMFLOAT3 right = RotateYVec(exitDoorRight_, angle);
        XMFLOAT3 normal = RotateYVec(exitDoorNormal_, angle);
        XMFLOAT3 center = Add3(exitDoorHinge_, Add3(Scale3(right, halfW), Scale3(normal, 0.018f)));
        AppendDynamicBoxAxes(verts, center, right, up, normal, {halfW, halfH, 0.026f}, 6.0f);

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

    void AppendMenuButtonPlaques(std::vector<Vertex>& verts, std::vector<Vertex>& transparentVerts) {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return;
        XMFLOAT3 c = maze_.WorldCenter(maze_.start, 0.0f);
        XMFLOAT3 right{1.0f, 0.0f, 0.0f};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
        float wallZ = c.z + maze_.tileD * 0.5f;
        float z = wallZ - 0.040f;
        float x = c.x + maze_.tileW * 0.14f;
        float startY = 1.50f;
        float bloodTopY = settings_.wallHeightMeters - 0.006f;
        float bloodBottomY = 0.34f;
        float bloodCenterY = (bloodTopY + bloodBottomY) * 0.5f;
        float bloodHalfHeight = std::max(0.40f, (bloodTopY - bloodBottomY) * 0.5f);
        XMFLOAT3 bloodCenter{x + 0.22f, bloodCenterY, wallZ - 0.002f};
        XMFLOAT3 bloodHalfW = Scale3(right, 0.74f);
        XMFLOAT3 bloodHalfH = Scale3(up, bloodHalfHeight);
        constexpr float kMenuWallBloodMaterial = 14.985f;
        constexpr float kMenuPoolBloodMaterial = 14.66f;
        AppendDynamicQuadUV(transparentVerts,
            Add3(bloodCenter, Add3(Scale3(bloodHalfW, -1.0f), Scale3(bloodHalfH, -1.0f))),
            Add3(bloodCenter, Add3(bloodHalfW, Scale3(bloodHalfH, -1.0f))),
            Add3(bloodCenter, Add3(bloodHalfW, bloodHalfH)),
            Add3(bloodCenter, Add3(Scale3(bloodHalfW, -1.0f), bloodHalfH)),
            inward, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, kMenuWallBloodMaterial);
        float ceilingDepth = std::min(maze_.tileD * 0.72f, 1.18f);
        float ceilingY = settings_.wallHeightMeters - 0.003f;
        XMFLOAT3 ceilingCenter{x + 0.22f, ceilingY, wallZ - ceilingDepth * 0.5f};
        XMFLOAT3 ceilingHalfD = Scale3(inward, ceilingDepth * 0.5f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(ceilingCenter, Add3(Scale3(bloodHalfW, -1.0f), Scale3(ceilingHalfD, -1.0f))),
            Add3(ceilingCenter, Add3(bloodHalfW, Scale3(ceilingHalfD, -1.0f))),
            Add3(ceilingCenter, Add3(bloodHalfW, ceilingHalfD)),
            Add3(ceilingCenter, Add3(Scale3(bloodHalfW, -1.0f), ceilingHalfD)),
            Scale3(up, -1.0f), right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, kMenuPoolBloodMaterial);
        float poolDepth = std::min(maze_.tileD * 0.58f, 0.94f);
        XMFLOAT3 poolCenter{x + 0.22f, 0.014f, wallZ - poolDepth * 0.42f};
        XMFLOAT3 poolHalfW = Scale3(right, 0.68f);
        XMFLOAT3 poolHalfD = Scale3(inward, poolDepth * 0.5f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(poolCenter, Add3(Scale3(poolHalfW, -1.0f), poolHalfD)),
            Add3(poolCenter, Add3(poolHalfW, poolHalfD)),
            Add3(poolCenter, Add3(poolHalfW, Scale3(poolHalfD, -1.0f))),
            Add3(poolCenter, Add3(Scale3(poolHalfW, -1.0f), Scale3(poolHalfD, -1.0f))),
            up, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, kMenuPoolBloodMaterial);
        for (int i = 0; i < 3; ++i) {
            bool hover = menuHoverButtonIndex_ == i;
            float y = startY - static_cast<float>(i) * 0.34f;
            float material = hover ? 10.72f : 9.70f;
            XMFLOAT3 plaqueCenter{x, y, z};
            AppendDynamicBoxAxes(verts, plaqueCenter, right, up, inward, {0.72f, 0.122f, 0.070f}, material);
            AppendDynamicBoxAxes(verts, {x - 0.71f, y, z + 0.033f}, right, up, inward, {0.018f, 0.134f, 0.048f}, 10.0f);
            XMFLOAT3 labelCenter = Add3(plaqueCenter, Scale3(inward, 0.070f));
            XMFLOAT3 hw = Scale3(right, 0.610f);
            XMFLOAT3 hh = Scale3(up, 0.081f);
            float v0 = (static_cast<float>(i) + 0.12f) / 3.0f;
            float v1 = (static_cast<float>(i) + 0.88f) / 3.0f;
            float labelMaterial = hover ? 18.65f : 18.08f;
            AppendDynamicQuadUV(transparentVerts,
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, hh)),
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), hh)),
                inward, right, {0.06f, v1}, {0.94f, v1}, {0.94f, v0}, {0.06f, v0}, labelMaterial);
        }
    }

    void AppendMonsterBillboard(std::vector<Vertex>& solidVerts, std::vector<Vertex>& transparentVerts) {
        float modelY = std::clamp(settings_.monsterScale, 0.35f, 1.25f);
        float modelXZ = std::clamp(settings_.monsterScale, 0.35f, 1.35f);
        float dist = MonsterDistance();
        bool canTrackPlayer = !monsterPreview_ && MonsterLineOfSightToPlayer();
        float faceYaw = monsterYaw_;
        if (canTrackPlayer) {
            float cameraYaw = std::atan2(camera_.x - monster_.x, camera_.z - monster_.z);
            faceYaw += AngleWrap(cameraYaw - faceYaw) * 0.42f;
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
        constexpr float darkMat = 10.0f;

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

        AppendDynamicEllipsoid(solidVerts, off(0.0f, 0.88f + breathe * 0.35f, kMonsterSmokeBackOffset * 0.34f),
            right, up, forward, {0.255f * modelXZ, 0.760f * modelY, 0.215f * modelXZ}, 18, 12, darkMat);
        AppendDynamicEllipsoid(solidVerts, off(0.0f, 1.26f + breathe * 0.25f, kMonsterSmokeBackOffset * 0.18f),
            right, up, forward, {0.205f * modelXZ, 0.530f * modelY, 0.178f * modelXZ}, 16, 10, darkMat);
        AppendDynamicEllipsoid(solidVerts, off(0.0f, 0.42f, kMonsterSmokeBackOffset * 0.58f),
            right, up, forward, {0.315f * modelXZ, 0.390f * modelY, 0.255f * modelXZ}, 16, 9, darkMat);

        constexpr int monsterSmokePuffs = 220;
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

        float liveLock = canTrackPlayer ? monsterHeadLockAmount_ : 0.0f;
        float headLock = std::max(deathHeadLock, liveLock);
        float scanWeight = 1.0f - Clamp01(headLock);
        float headYaw = faceYaw + monsterHeadYawOffset_ * scanWeight +
            twitch * (1.0f - deathHeadLock * 0.85f) * scanWeight;
        XMFLOAT3 hRight{std::cos(headYaw), 0.0f, -std::sin(headYaw)};
        XMFLOAT3 hUp = up;
        XMFLOAT3 hForward{std::sin(headYaw), 0.0f, std::cos(headYaw)};
        XMFLOAT3 skull = off(0.0f, 2.00f + MonsterHeadBobOffset(), kMonsterHeadForwardOffset);
        float headPitch = monsterHeadPitchOffset_ * scanWeight;
        if (std::abs(headPitch) > 0.0005f) {
            hForward = Normalize3(Add3(Scale3(hForward, std::cos(headPitch)), Scale3(hUp, std::sin(headPitch))), hForward);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        if (headLock > 0.001f) {
            XMFLOAT3 cameraFocus{camera_.x, camera_.y + 0.04f, camera_.z};
            XMFLOAT3 lookForward = Normalize3(Sub3(cameraFocus, skull), hForward);
            hForward = Normalize3(Lerp3(hForward, lookForward, Clamp01(headLock)), lookForward);
            hRight = Normalize3(Cross3(up, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), up);
        }
        bool externalSkull = AppendExternalSkullMesh(solidVerts, skull, hRight, hUp, hForward, modelXZ, modelY);
        if (!externalSkull) {
            AppendDynamicEllipsoid(solidVerts, skull, hRight, hUp, hForward,
                {0.178f * modelXZ, 0.162f * modelY, 0.145f * modelXZ}, 18, 10, boneMat);
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Add3(Scale3(hForward, 0.190f * modelXZ), Scale3(hUp, -0.020f * modelY))), hRight, hUp, hForward,
                {0.108f * modelXZ, 0.074f * modelY, 0.196f * modelXZ}, 16, 8, boneMat);
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Add3(Scale3(hForward, 0.318f * modelXZ), Scale3(hUp, -0.090f * modelY))), hRight, hUp, hForward,
                {0.126f * modelXZ, 0.040f * modelY, 0.056f * modelXZ}, 14, 6, darkMat);
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Add3(Scale3(hForward, 0.075f * modelXZ), Scale3(hUp, -0.214f * modelY))), hRight, hUp, hForward,
                {0.058f * modelXZ, 0.152f * modelY, 0.050f * modelXZ}, 12, 7, boneMat);
            for (int side = -1; side <= 1; side += 2) {
                XMFLOAT3 cheekA = Add3(skull, OrientedOffset(hRight, hUp, hForward, 0.120f * side * modelXZ, -0.030f * modelY, 0.110f * modelXZ));
                XMFLOAT3 cheekB = Add3(skull, OrientedOffset(hRight, hUp, hForward, 0.215f * side * modelXZ, -0.120f * modelY, 0.220f * modelXZ));
                seg(cheekA, cheekB, 0.012f, 0.010f, boneMat);
            }
        }

        if (!externalSkull) {
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
            float eyeHalfW = externalSkull ? 0.074f : 0.055f;
            float eyeHalfH = externalSkull ? 0.052f : 0.040f;
            XMFLOAT3 center = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                xOffset * modelXZ, eyeUp * modelY, eyeForward * modelXZ));
            XMFLOAT3 eyeNormal = hForward;
            XMFLOAT3 eyeRight = hRight;
            XMFLOAT3 eyeUpAxis = hUp;
            if (externalSkull) {
                float sideSign = xOffset >= 0.0f ? 1.0f : -1.0f;
                eyeNormal = Normalize3(Add3(Scale3(hRight, sideSign * 0.42f), Scale3(hForward, 0.90f)), hForward);
                eyeRight = Normalize3(Cross3(hUp, eyeNormal), hRight);
                eyeUpAxis = hUp;
                AppendDynamicEllipsoid(solidVerts, center, eyeRight, eyeUpAxis, eyeNormal,
                    {0.045f * modelXZ, 0.034f * modelY, 0.028f * modelXZ}, 12, 7, 10.70f + variant * 0.05f);
            }
            XMFLOAT3 ew = Scale3(eyeRight, eyeHalfW * modelXZ);
            XMFLOAT3 eh = Scale3(eyeUpAxis, eyeHalfH * modelY);
            AppendDynamicQuad(transparentVerts,
                Add3(center, Add3(Scale3(ew, -1.0f), Scale3(eh, -1.0f))),
                Add3(center, Add3(ew, Scale3(eh, -1.0f))),
                Add3(center, Add3(ew, eh)),
                Add3(center, Add3(Scale3(ew, -1.0f), eh)),
                eyeNormal, eyeRight, 12.05f + variant);
        };
        if (externalSkull) {
            appendEye(monsterUsingAltSkull_ ? settings_.monsterAltRightEyeX : settings_.monsterRightEyeX,
                monsterUsingAltSkull_ ? settings_.monsterAltRightEyeY : settings_.monsterRightEyeY,
                monsterUsingAltSkull_ ? settings_.monsterAltRightEyeZ : settings_.monsterRightEyeZ,
                0.14f);
            appendEye(monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeX : settings_.monsterLeftEyeX,
                monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeY : settings_.monsterLeftEyeY,
                monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeZ : settings_.monsterLeftEyeZ,
                0.34f);
        } else {
            appendEye(-0.060f, 0.025f, 0.162f, 0.14f);
            appendEye(0.060f, 0.025f, 0.162f, 0.34f);
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
            float halfW = size * (0.82f + p.aspect * 0.10f);
            float halfH = size * (0.82f + (1.0f / std::max(0.60f, p.aspect)) * 0.10f);
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
