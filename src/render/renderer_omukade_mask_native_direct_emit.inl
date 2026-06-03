            auto pushFaceVertex = [&](const FaceV& fv) {
                XMFLOAT3 p = localPos(fv.p);
                XMFLOAT3 n = (fv.n >= 0 && fv.n < static_cast<int>(normals.size()))
                    ? localNormal(fv.n)
                    : Normalize3(p, {0.0f, 0.0f, 1.0f});
                XMFLOAT2 uv = (fv.t >= 0 && fv.t < static_cast<int>(texcoords.size()))
                    ? texcoords[static_cast<size_t>(fv.t)]
                    : XMFLOAT2{0.5f + p.x * 0.5f, 0.5f - p.y * 0.5f};
                XMFLOAT3 tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
                out.push_back({p, n, tangent, uv, 26.0f});
            };
            if (faceTris.size() <= static_cast<size_t>(maxTriangles)) {
                out.clear();
                out.reserve(faceTris.size() * 3);
                for (const auto& tri : faceTris) {
                    pushFaceVertex(tri[0]);
                    pushFaceVertex(tri[1]);
                    pushFaceVertex(tri[2]);
                }
                return !out.empty();
            }
