        if (!gEffectDebugViewer || gDebugSliceEffect != DebugSliceEffect::Props) return;
        int propIndex = WrapDebugPropIndex(gDebugPropIndex);
        const StaticPropMesh* mesh = nullptr;
        switch (propIndex) {
        case 0: mesh = &renderAssets_.chairPropMeshes[0]; break;
        case 1: mesh = &renderAssets_.chairPropMeshes[1]; break;
        case 2:
        case 3: mesh = &renderAssets_.chairPropMeshes[2]; break;
        case 4: mesh = &renderAssets_.cabinetPropMesh; break;
        case 5: mesh = &renderAssets_.deskPropMesh; break;
        case 6:
        case 7: mesh = &renderAssets_.trashBinPropMesh; break;
        case 8: mesh = &renderAssets_.deskLampPropMesh; break;
        case 9: mesh = &renderAssets_.cassettePropMesh; break;
        case 10: mesh = &renderAssets_.airVentPropMesh; break;
        case 11: mesh = &renderAssets_.exitSignPropMesh; break;
        case 12: mesh = &renderAssets_.ceilingLampPropMeshes[0]; break;
        case 13: mesh = &renderAssets_.ceilingLampPropMeshes[1]; break;
        case 14: mesh = &renderAssets_.ceilingLampPropMeshes[2]; break;
        case 15: mesh = &renderAssets_.ceilingLampPropMeshes[3]; break;
        default: break;
        }
