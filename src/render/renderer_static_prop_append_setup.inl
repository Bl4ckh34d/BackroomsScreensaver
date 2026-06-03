    bool AppendStaticPropMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                              const StaticPropMesh& mesh, XMFLOAT3 origin, float yaw,
                              float scaleX, float scaleY, float scaleZ,
                              float pitch = 0.0f, float materialOverride = -1.0f,
                              std::vector<uint32_t>* shadowIndices = nullptr,
                              float materialVariant = 0.0f) const {
        if (mesh.vertices.empty()) return false;
        if (vertices.size() + mesh.vertices.size() > static_cast<size_t>(std::numeric_limits<uint32_t>::max())) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);
        const float invX = 1.0f / std::max(0.001f, std::abs(scaleX));
        const float invY = 1.0f / std::max(0.001f, std::abs(scaleY));
        const float invZ = 1.0f / std::max(0.001f, std::abs(scaleZ));
        XMFLOAT3 meshSpan{
            std::max(0.001f, mesh.max.x - mesh.min.x),
            std::max(0.001f, mesh.max.y - mesh.min.y),
            std::max(0.001f, mesh.max.z - mesh.min.z)
        };
        auto generatedUv = [&](const Vertex& src) {
            float nx = (src.pos.x - mesh.min.x) / meshSpan.x;
            float ny = (src.pos.y - mesh.min.y) / meshSpan.y;
            float nz = (src.pos.z - mesh.min.z) / meshSpan.z;
            XMFLOAT3 nAbs{std::abs(src.normal.x), std::abs(src.normal.y), std::abs(src.normal.z)};
            XMFLOAT2 uv = nAbs.y >= nAbs.x && nAbs.y >= nAbs.z
                ? XMFLOAT2{nx, nz}
                : (nAbs.x >= nAbs.z ? XMFLOAT2{nz, 1.0f - ny} : XMFLOAT2{nx, 1.0f - ny});
            int materialId = std::clamp(static_cast<int>(std::floor(src.material)), 0, kMaterialCount - 1);
            if (materialId == 22) {
                uv = {0.20f + uv.x * 0.60f, 0.28f + uv.y * 0.36f};
            }
            return uv;
        };
