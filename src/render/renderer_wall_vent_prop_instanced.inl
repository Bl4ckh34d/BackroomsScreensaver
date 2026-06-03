        if (!renderAssets_.airVentPropMesh.vertices.empty()) {
            float spanX = std::max(0.001f, PropMeshSpan(renderAssets_.airVentPropMesh, 0));
            float spanY = std::max(0.001f, PropMeshSpan(renderAssets_.airVentPropMesh, 1));
            float scale = std::min(ventW / spanX, ventH / spanY);
            scale = std::clamp(scale, 0.05f, 8.0f);
            if (AddInstancedStaticProp(renderAssets_.airVentPropMesh, center, yaw, scale, scale, scale,
                    instancedVertices, instancedIndices, pendingStaticInstances, instancedMeshRanges)) {
