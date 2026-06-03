    int RegisterInstancedStaticMesh(const StaticPropMesh& mesh,
                                    std::vector<Vertex>& instancedVertices,
                                    std::vector<uint32_t>& instancedIndices,
                                    std::vector<InstancedMeshRange>& instancedMeshRanges) const {
        if (mesh.vertices.empty()) return -1;
        for (size_t i = 0; i < instancedMeshRanges.size(); ++i) {
            if (instancedMeshRanges[i].mesh == &mesh) return static_cast<int>(i);
        }
        if (instancedVertices.size() + mesh.vertices.size() > static_cast<size_t>(std::numeric_limits<uint32_t>::max())) return -1;

        InstancedMeshRange range{};
        range.mesh = &mesh;
        range.baseVertex = static_cast<INT>(instancedVertices.size());
        range.startIndex = static_cast<UINT>(instancedIndices.size());
        range.indexCount = static_cast<UINT>(mesh.vertices.size());
        range.min = mesh.min;
        range.max = mesh.max;

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

        instancedVertices.reserve(instancedVertices.size() + mesh.vertices.size());
        instancedIndices.reserve(instancedIndices.size() + mesh.vertices.size());
        for (const Vertex& src : mesh.vertices) {
            Vertex out = src;
            if (mesh.generatedUvFallback) {
                out.uv = generatedUv(src);
            }
            instancedVertices.push_back(out);
        }
        for (UINT n = 0; n < range.indexCount; ++n) {
            instancedIndices.push_back(n);
        }
        instancedMeshRanges.push_back(range);
        return static_cast<int>(instancedMeshRanges.size() - 1);
    }
