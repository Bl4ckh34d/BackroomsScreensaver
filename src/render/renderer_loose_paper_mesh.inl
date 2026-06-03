    static constexpr float kLoosePaperShortMeters = 0.210f;
    static constexpr float kLoosePaperLongMeters = 0.297f;

    static StaticPropMesh BuildLoosePaperInstancedMesh() {
        StaticPropMesh mesh;
        const float hw = kLoosePaperShortMeters * 0.5f;
        const float hd = kLoosePaperLongMeters * 0.5f;
        mesh.min = {-hw, 0.0f, -hd};
        mesh.max = { hw, 0.0f,  hd};
        mesh.vertices = {
            {{-hw, 0.0f,  hd}, {0, 1, 0}, {1, 0, 0}, {0, 0}, static_cast<float>(kRandomLoosePageMaterial)},
            {{ hw, 0.0f,  hd}, {0, 1, 0}, {1, 0, 0}, {1, 0}, static_cast<float>(kRandomLoosePageMaterial)},
            {{ hw, 0.0f, -hd}, {0, 1, 0}, {1, 0, 0}, {1, 1}, static_cast<float>(kRandomLoosePageMaterial)},
            {{-hw, 0.0f,  hd}, {0, 1, 0}, {1, 0, 0}, {0, 0}, static_cast<float>(kRandomLoosePageMaterial)},
            {{ hw, 0.0f, -hd}, {0, 1, 0}, {1, 0, 0}, {1, 1}, static_cast<float>(kRandomLoosePageMaterial)},
            {{-hw, 0.0f, -hd}, {0, 1, 0}, {1, 0, 0}, {0, 1}, static_cast<float>(kRandomLoosePageMaterial)}
        };
        return mesh;
    }
