// Renderer geometry primitive helpers.
// Included inside Renderer private section before mesh construction helpers.

    void AddQuad(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                 XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                 XMFLOAT3 n, XMFLOAT3 t, float material, float uScale, float vScale) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, {0.0f, 0.0f}, material});
        v.push_back({b, n, t, {uScale, 0.0f}, material});
        v.push_back({c, n, t, {uScale, vScale}, material});
        v.push_back({d, n, t, {0.0f, vScale}, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    void AddQuadUV(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                   XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                   XMFLOAT3 n, XMFLOAT3 t, XMFLOAT2 auv, XMFLOAT2 buv, XMFLOAT2 cuv, XMFLOAT2 duv,
                   float material) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, auv, material});
        v.push_back({b, n, t, buv, material});
        v.push_back({c, n, t, cuv, material});
        v.push_back({d, n, t, duv, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    static XMFLOAT3 Add3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    static XMFLOAT3 Scale3(XMFLOAT3 a, float s) {
        return {a.x * s, a.y * s, a.z * s};
    }

    static XMFLOAT3 Sub3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    static float Dot3(XMFLOAT3 a, XMFLOAT3 b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static XMFLOAT3 Cross3(XMFLOAT3 a, XMFLOAT3 b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static float Length3(XMFLOAT3 a) {
        return std::sqrt(std::max(0.0f, Dot3(a, a)));
    }

    static XMFLOAT3 Normalize3(XMFLOAT3 a, XMFLOAT3 fallback = {0.0f, 1.0f, 0.0f}) {
        float len = Length3(a);
        if (len <= 0.0001f) return fallback;
        return Scale3(a, 1.0f / len);
    }

    static XMFLOAT3 Lerp3(XMFLOAT3 a, XMFLOAT3 b, float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t)};
    }

    static XMFLOAT3 RotateYVec(XMFLOAT3 a, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return {a.x * c + a.z * s, a.y, -a.x * s + a.z * c};
    }

    static XMFLOAT3 OrientedOffset(XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward, float x, float y, float z) {
        return Add3(Add3(Scale3(right, x), Scale3(up, y)), Scale3(forward, z));
    }

    void AddOrientedBox(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                        XMFLOAT3 center, XMFLOAT3 half, float yaw, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        auto p = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, forward, x * half.x, y * half.y, z * half.z));
        };
        auto face = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c0, XMFLOAT3 d, XMFLOAT3 n, XMFLOAT3 t) {
            AddQuadUV(v, i, a, b, c0, d, n, t, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
        };
        face(p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right);
        face(p( 1, -1, -1), p(-1, -1, -1), p(-1,  1, -1), p( 1,  1, -1), Scale3(forward, -1.0f), Scale3(right, -1.0f));
        face(p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f));
        face(p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward);
        face(p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right);
        face(p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right);
    }

    void AddFloorCard(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                      XMFLOAT3 center, float width, float depth, float yaw, float y, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 d = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward, -depth * 0.5f)));
        a.y = b.y = c0.y = d.y = y;
        AddQuadUV(v, i, a, b, c0, d, {0, 1, 0}, right, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
    }

    void AddCeilingCard(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                        XMFLOAT3 center, float width, float depth, float yaw, float y, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 d = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward,  depth * 0.5f)));
        a.y = b.y = c0.y = d.y = y;
        AddQuadUV(v, i, a, b, c0, d, {0, -1, 0}, right, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
    }
