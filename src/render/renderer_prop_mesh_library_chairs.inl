        int loaded = 0;
        const std::array<std::wstring, 3> chairPaths = {
            L"assets\\models\\runtime\\office_chair_modern.brmesh",
            L"assets\\models\\runtime\\office_chair_classic.brmesh",
            L"assets\\models\\runtime\\office_chair_task.brmesh"
        };
        const std::array<float, 3> chairMaterials = {16.0f, 20.0f, 22.0f};
        for (size_t i = 0; i < chairPaths.size(); ++i) {
            if (LoadStaticPropObj(chairPaths[i], chairMaterials[i], renderAssets_.chairPropMeshes[i])) {
                for (Vertex& v : renderAssets_.chairPropMeshes[i].vertices) {
                    int materialId = static_cast<int>(std::floor(v.material));
                    if (materialId == 19 || materialId == 20 || materialId == 21) {
                        v.material = chairMaterials[i];
                    }
                }
                ++loaded;
            }
        }
