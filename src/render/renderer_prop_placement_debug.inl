// Static prop placement debug inspection.

    void AddDebugPropInspectionModel(std::vector<Vertex>& vertices,
                                     std::vector<uint32_t>& indices,
                                     std::vector<uint32_t>& propShadowIndices,
                                     const MazeSurfaceBuildContext& ctx) {
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

        int tiles = std::clamp(gDebugSliceTiles, 1, 5);
        float centerX = ctx.ox + (1.0f + static_cast<float>(tiles) * 0.5f) * ctx.tileW;
        float centerZ = ctx.oz + (1.0f + static_cast<float>(tiles) * 0.5f) * ctx.tileD;
        float targetMax = 1.22f;
        float yaw = kPi;
        float pitch = 0.0f;
        switch (propIndex) {
        case 3:
            pitch = kPi * 0.5f;
            targetMax = 1.12f;
            break;
        case 6:
            targetMax = 0.58f;
            break;
        case 7:
            pitch = kPi * 0.5f;
            targetMax = 0.62f;
            break;
        case 4:
            targetMax = 1.44f;
            break;
        case 5:
            targetMax = 1.62f;
            yaw = kPi * 0.5f;
            break;
        case 8:
            targetMax = 0.56f;
            break;
        case 9:
            targetMax = 0.58f;
            break;
        case 10:
            targetMax = 0.86f;
            break;
        case 11:
            targetMax = 1.18f;
            break;
        case 12:
        case 13:
        case 14:
        case 15:
            targetMax = 1.36f;
            yaw = kPi * 0.5f;
            break;
        default:
            break;
        }

        if (mesh && !mesh->vertices.empty()) {
            float spanX = PropMeshSpan(*mesh, 0);
            float spanY = PropMeshSpan(*mesh, 1);
            float spanZ = PropMeshSpan(*mesh, 2);
            float spanMax = std::max(spanX, std::max(spanY, spanZ));
            float scale = std::clamp(targetMax / std::max(0.001f, spanMax), 0.035f, 12.0f);
            bool wallMounted = propIndex == 10 || propIndex == 11;
            bool suspendedLamp = propIndex >= 12 && propIndex <= 15;
            if (wallMounted || suspendedLamp) {
                float c = std::cos(yaw);
                float s = std::sin(yaw);
                XMFLOAT3 right{c, 0.0f, -s};
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 forward{s, 0.0f, c};
                XMFLOAT3 meshCenter{
                    (mesh->min.x + mesh->max.x) * 0.5f,
                    (mesh->min.y + mesh->max.y) * 0.5f,
                    (mesh->min.z + mesh->max.z) * 0.5f
                };
                XMFLOAT3 desiredCenter{
                    centerX,
                    wallMounted ? ctx.wallH * 0.54f : 1.08f,
                    wallMounted ? centerZ - ctx.tileD * 0.34f : centerZ
                };
                XMFLOAT3 centeredOffset = Add3(Scale3(right, meshCenter.x * scale),
                    Add3(Scale3(up, meshCenter.y * scale), Scale3(forward, meshCenter.z * scale)));
                XMFLOAT3 origin = Add3(desiredCenter, Scale3(centeredOffset, -1.0f));
                AppendStaticPropMesh(vertices, indices, *mesh, origin, yaw, scale, scale, scale,
                    0.0f, -1.0f, &propShadowIndices);
                cameraRuntime_.propLookPoints.push_back(desiredCenter);
                return;
            }
            AppendStaticPropMeshGrounded(vertices, indices, *mesh, {centerX, 0.0f, centerZ},
                yaw, scale, scale, scale, pitch, -1.0f, &propShadowIndices);
            float lookY = std::clamp((mesh->max.y - mesh->min.y) * scale * 0.55f, 0.16f, 1.15f);
            cameraRuntime_.propLookPoints.push_back({centerX, lookY, centerZ});
            return;
        }

        if (propIndex == 10) {
            float panelY = ctx.wallH * 0.54f;
            float panelZ = centerZ - ctx.tileD * 0.34f;
            AddOrientedBox(vertices, indices, {centerX, panelY, panelZ}, {0.52f, 0.18f, 0.018f}, kPi, 10.0f);
            AddOrientedBox(vertices, indices, {centerX, panelY, panelZ - 0.026f}, {0.42f, 0.11f, 0.010f}, kPi, 5.0f);
            for (int slot = -3; slot <= 3; ++slot) {
                AddOrientedBox(vertices, indices,
                    {centerX, panelY + static_cast<float>(slot) * 0.030f, panelZ - 0.044f},
                    {0.36f, 0.0048f, 0.006f}, kPi, 8.0f);
            }
            cameraRuntime_.propLookPoints.push_back({centerX, panelY, panelZ});
            return;
        }

        AddOrientedBox(vertices, indices, {centerX, 0.18f, centerZ}, {0.42f, 0.18f, 0.42f}, 0.0f, 5.0f);
        cameraRuntime_.propLookPoints.push_back({centerX, 0.18f, centerZ});
    }
