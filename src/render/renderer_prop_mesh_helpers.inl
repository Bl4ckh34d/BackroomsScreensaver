// Prop mesh selection and sizing helpers.
// Included inside Renderer private section before maze mesh construction.

    static float PropMeshSpan(const StaticPropMesh& mesh, int axis) {
        if (axis == 0) return std::max(0.001f, mesh.max.x - mesh.min.x);
        if (axis == 1) return std::max(0.001f, mesh.max.y - mesh.min.y);
        return std::max(0.001f, mesh.max.z - mesh.min.z);
    }

    static XMFLOAT3 CabinetPropSize(bool tall) {
        return tall
            ? XMFLOAT3{0.66f, 1.34f, 0.40f}
            : XMFLOAT3{0.78f, 0.92f, 0.42f};
    }

    const StaticPropMesh* PickChairPropMesh(float seed, bool waitingChair) const {
        int order[3] = {0, 1, 2};
        if (waitingChair) {
            order[0] = 1;
            order[1] = 0;
            order[2] = 2;
        } else {
            order[0] = 2;
            order[1] = 0;
            order[2] = 1;
        }
        std::array<const StaticPropMesh*, 3> candidates{};
        int count = 0;
        for (int idx : order) {
            if (!renderAssets_.chairPropMeshes[static_cast<size_t>(idx)].vertices.empty()) {
                candidates[static_cast<size_t>(count++)] = &renderAssets_.chairPropMeshes[static_cast<size_t>(idx)];
            }
        }
        if (count <= 0) return nullptr;
        int pick = std::clamp(static_cast<int>(seed * static_cast<float>(count)), 0, count - 1);
        return candidates[static_cast<size_t>(pick)];
    }
