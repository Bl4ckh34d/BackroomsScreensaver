// Dynamic monster skull mesh transform helpers. 
// Included inside Renderer's private section from renderer_dynamic_geometry.inl.

    XMFLOAT3 ActiveSkullRotationDegrees() const {
        return renderAssets_.monsterUsingAltSkull
            ? XMFLOAT3{settingsRuntime_.live.monsterAltSkullPitchDegrees, settingsRuntime_.live.monsterAltSkullYawDegrees, settingsRuntime_.live.monsterAltSkullRollDegrees}
            : XMFLOAT3{settingsRuntime_.live.monsterSkullPitchDegrees, settingsRuntime_.live.monsterSkullYawDegrees, settingsRuntime_.live.monsterSkullRollDegrees};
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
        if (renderAssets_.skullMesh.empty()) return false;
        if (verts.size() + renderAssets_.skullMesh.size() + 512 > kDynamicVertexCapacity) return false;
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        XMFLOAT3 skullAxisX = RotateSkullLocalVector({1.0f, 0.0f, 0.0f});
        XMFLOAT3 skullAxisY = RotateSkullLocalVector({0.0f, 1.0f, 0.0f});
        XMFLOAT3 skullAxisZ = RotateSkullLocalVector({0.0f, 0.0f, 1.0f});
        auto rotateLocal = [&](XMFLOAT3 v) {
            return Add3(Scale3(skullAxisX, v.x),
                Add3(Scale3(skullAxisY, v.y), Scale3(skullAxisZ, v.z)));
        };
        for (const Vertex& src : renderAssets_.skullMesh) {
            XMFLOAT3 baseLocal = src.pos;
            XMFLOAT3 baseNormal = src.normal;
            XMFLOAT3 baseTangent = src.tangent;
            XMFLOAT3 local = rotateLocal(baseLocal);
            XMFLOAT3 nLocal = rotateLocal(baseNormal);
            XMFLOAT3 tLocal = rotateLocal(baseTangent);
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
