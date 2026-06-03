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
