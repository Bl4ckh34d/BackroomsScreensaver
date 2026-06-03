        out.clear();
        out.reserve(tris.size() * 3);
        auto pushVertex = [&](int cluster, XMFLOAT2 uv) {
            const Cluster& c = clusters[static_cast<size_t>(cluster)];
            XMFLOAT3 p = c.count > 0 ? Scale3(c.sum, 1.0f / static_cast<float>(c.count)) : XMFLOAT3{};
            XMFLOAT3 radial = Normalize3({p.x * 0.86f, p.y * 1.18f, p.z * 0.86f}, Normalize3(p, {0.0f, 1.0f, 0.0f}));
            XMFLOAT3 averaged = Normalize3(c.normal, radial);
            if (Dot3(averaged, radial) < 0.18f) averaged = radial;
            XMFLOAT3 n = Normalize3(Lerp3(averaged, radial, 0.62f), radial);
            XMFLOAT3 tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
            out.push_back({p, n, tangent, uv, 9.65f});
        };
        for (const Tri& tri : tris) {
            pushVertex(tri.a, {0.0f, 0.0f});
            pushVertex(tri.b, {1.0f, 0.0f});
            pushVertex(tri.c, {0.5f, 1.0f});
        }
        return !out.empty();
