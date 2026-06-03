    bool AppendStaticPropMeshGrounded(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                                      const StaticPropMesh& mesh, XMFLOAT3 floorCenter, float yaw,
                                      float scaleX, float scaleY, float scaleZ,
                                      float pitch = 0.0f, float materialOverride = -1.0f,
                                      std::vector<uint32_t>* shadowIndices = nullptr,
                                      float materialVariant = 0.0f) const {
        if (mesh.vertices.empty()) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);

        float minY = std::numeric_limits<float>::max();
        for (const Vertex& src : mesh.vertices) {
            float y = right.y * src.pos.x * scaleX + up.y * src.pos.y * scaleY + forward.y * src.pos.z * scaleZ;
            minY = std::min(minY, y);
        }
        XMFLOAT3 origin{floorCenter.x, floorCenter.y - minY, floorCenter.z};
        return AppendStaticPropMesh(vertices, indices, mesh, origin, yaw, scaleX, scaleY, scaleZ, pitch, materialOverride, shadowIndices, materialVariant);
    }

