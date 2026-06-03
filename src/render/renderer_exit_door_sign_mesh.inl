        if (!renderAssets_.exitSignPropMesh.vertices.empty()) {
            float spanX = std::max(0.001f, PropMeshSpan(renderAssets_.exitSignPropMesh, 0));
            float spanY = std::max(0.001f, PropMeshSpan(renderAssets_.exitSignPropMesh, 1));
            float targetH = fixedSignTargetH;
            float scale = std::clamp(std::min(targetW / spanX, targetH / spanY), 0.05f, 8.0f);
            float actualSignW = spanX * scale;
            float actualSignH = spanY * scale;
            AddOrientedBox(build.vertices, build.indices, Add3(sign, Scale3(forward, -0.004f)),
                {actualSignW * 0.5f + 0.024f, actualSignH * 0.5f + 0.016f, 0.012f}, exitPortal.yaw, 10.0f);
            XMFLOAT3 localCenter{
                (renderAssets_.exitSignPropMesh.min.x + renderAssets_.exitSignPropMesh.max.x) * 0.5f,
                (renderAssets_.exitSignPropMesh.min.y + renderAssets_.exitSignPropMesh.max.y) * 0.5f,
                (renderAssets_.exitSignPropMesh.min.z + renderAssets_.exitSignPropMesh.max.z) * 0.5f
            };
            auto appendExitSignModel = [&](XMFLOAT3 signCenter, XMFLOAT3 signRight, XMFLOAT3 signForward, float yaw) {
                XMFLOAT3 origin = Add3(signCenter, Add3(Scale3(signRight, -localCenter.x * scale),
                    Add3(Scale3(up, -localCenter.y * scale), Scale3(signForward, -localCenter.z * scale))));
                return AddInstancedStaticProp(renderAssets_.exitSignPropMesh, origin, yaw, scale, scale, scale,
                    build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
                    0.0f, 7.0f);
            };
            bool appended = appendExitSignModel(Add3(sign, Scale3(forward, 0.012f)), right, forward, exitPortal.yaw);
            if (!appended) {
                StartupProfileLine(L"Emergency exit sign mesh was loaded but could not be appended; no handmade fallback was drawn.");
            }
        } else {
            AddOrientedBox(build.vertices, build.indices, Add3(sign, Scale3(forward, 0.012f)),
                {targetW * 0.5f, fixedSignTargetH * 0.5f, 0.014f}, exitPortal.yaw, 7.0f);
            StartupProfileLine(L"Emergency exit sign mesh missing; using handmade fallback.");
        }
