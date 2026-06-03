        auto uvFor = [&](XMFLOAT3 p, XMFLOAT3 n, int texcoord) {
            if (texcoord >= 0 && texcoord < static_cast<int>(texcoords.size())) {
                return texcoords[static_cast<size_t>(texcoord)];
            }
            if (std::abs(n.y) > 0.62f) return XMFLOAT2{p.x * 1.7f + 0.5f, p.z * 1.7f + 0.5f};
            if (std::abs(n.x) > std::abs(n.z)) return XMFLOAT2{p.z * 1.7f + 0.5f, p.y * 1.7f};
            return XMFLOAT2{p.x * 1.7f + 0.5f, p.y * 1.7f};
        };
        auto addTri = [&](ObjFaceVertex va, ObjFaceVertex vb, ObjFaceVertex vc, float faceMaterial) {
            XMFLOAT3 a = positions[static_cast<size_t>(va.vertex)];
            XMFLOAT3 b = positions[static_cast<size_t>(vb.vertex)];
            XMFLOAT3 c = positions[static_cast<size_t>(vc.vertex)];
            XMFLOAT3 n = Normalize3(Cross3(Sub3(b, a), Sub3(c, a)), {0.0f, 1.0f, 0.0f});
            XMFLOAT3 tangent = Normalize3(Sub3(b, a), {1.0f, 0.0f, 0.0f});
            if (std::abs(Dot3(tangent, n)) > 0.92f) {
                tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
            }
            out.vertices.push_back({a, n, tangent, uvFor(a, n, va.texcoord), faceMaterial});
            out.vertices.push_back({b, n, tangent, uvFor(b, n, vb.texcoord), faceMaterial});
            out.vertices.push_back({c, n, tangent, uvFor(c, n, vc.texcoord), faceMaterial});
        };
