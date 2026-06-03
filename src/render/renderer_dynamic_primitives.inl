// Dynamic geometry primitive append helpers. 
// Included inside Renderer's private section from renderer_dynamic_geometry.inl.

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

    bool DynamicVisualCandidate(const XMFLOAT3& pos, float radius, float maxDistance) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float dx = pos.x - world.playerPosition.x;
        float dy = pos.y - world.playerPosition.y;
        float dz = pos.z - world.playerPosition.z;
        float padded = std::max(0.1f, maxDistance + radius);
        if (dx * dx + dy * dy + dz * dz > padded * padded) return false;
        XMFLOAT3 forward = DirectionFromYawPitch(world.playerYaw, world.playerPitch);
        float depth = dx * forward.x + dy * forward.y + dz * forward.z;
        return depth > -radius * 2.0f;
    }

    bool DynamicBillboardVisible(const XMFLOAT3& pos, float radius, float maxDistance, float minProjectedPixels) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 to = Sub3(pos, world.playerPosition);
        float distSq = Dot3(to, to);
        float padded = std::max(0.1f, maxDistance + radius);
        if (distSq > padded * padded) return false;
        XMFLOAT3 forward = DirectionFromYawPitch(world.playerYaw, world.playerPitch);
        float depth = Dot3(to, forward);
        if (depth <= -radius * 2.0f) return false;
        float projectedPixels = (radius / std::max(0.08f, std::abs(depth))) *
            static_cast<float>(std::max<LONG>(1, hostRuntime_.height)) * 0.72f;
        return projectedPixels >= minProjectedPixels;
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
