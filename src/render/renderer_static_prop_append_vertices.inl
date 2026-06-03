        uint32_t base = static_cast<uint32_t>(vertices.size());
        vertices.reserve(vertices.size() + mesh.vertices.size());
        indices.reserve(indices.size() + mesh.vertices.size());
        for (const Vertex& src : mesh.vertices) {
            XMFLOAT3 pos = Add3(origin, Add3(Scale3(right, src.pos.x * scaleX),
                Add3(Scale3(up, src.pos.y * scaleY), Scale3(forward, src.pos.z * scaleZ))));
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, src.normal.x * invX),
                Add3(Scale3(up, src.normal.y * invY), Scale3(forward, src.normal.z * invZ))), up);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, src.tangent.x * scaleX),
                Add3(Scale3(up, src.tangent.y * scaleY), Scale3(forward, src.tangent.z * scaleZ))), right);
            XMFLOAT2 uv = mesh.generatedUvFallback ? generatedUv(src) : src.uv;
            float outMaterial = materialOverride >= 0.0f ? materialOverride : src.material;
            int materialId = std::clamp(static_cast<int>(std::floor(outMaterial)), 0, kMaterialCount - 1);
            if (materialOverride < 0.0f && materialId == 22) {
                outMaterial = 22.020f + std::fmod(std::abs(materialVariant), 0.92f);
            }
            vertices.push_back({pos, normal, tangent, uv, outMaterial});
        }
