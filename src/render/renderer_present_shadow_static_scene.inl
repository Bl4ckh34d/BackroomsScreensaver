            if (!monsterPreview_.active) {
                float shadowTileAverage = visibilityMaze ? visibilityMaze->TileAverage() : 1.0f;
                if (includeStaticMazeCasters) {
                    if (!staticSceneGeometry_.opaqueChunks.empty() || !staticSceneGeometry_.floorCeilingChunks.empty()) {
                        StartupProfileLine(L"Render before ShadowStatic chunked DrawIndexed");
                        UINT drawn = drawVisibleChunks(staticSceneGeometry_.opaqueChunks, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                            shadowTileAverage * 1.4f, false, 1);
                        drawn += drawVisibleChunks(staticSceneGeometry_.floorCeilingChunks, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                            shadowTileAverage * 1.4f, false, 1);
                        if (drawn == 0) {
                            UINT shadowStaticIndexCount = staticSceneGeometry_.waterStartIndex > 0 ? staticSceneGeometry_.waterStartIndex :
                                (staticSceneGeometry_.transparentStartIndex > 0 ? staticSceneGeometry_.transparentStartIndex : staticSceneGeometry_.indexCount);
                            d3dRuntime_.context->DrawIndexed(shadowStaticIndexCount, 0, 0);
                        }
                        renderProfile.Mark(L"ShadowStatic");
                    } else {
                        UINT shadowStaticIndexCount = staticSceneGeometry_.waterStartIndex > 0 ? staticSceneGeometry_.waterStartIndex :
                            (staticSceneGeometry_.transparentStartIndex > 0 ? staticSceneGeometry_.transparentStartIndex : staticSceneGeometry_.indexCount);
                        if (shadowStaticIndexCount > 0) {
                            StartupProfileLine(L"Render before ShadowStatic DrawIndexed");
                            d3dRuntime_.context->DrawIndexed(shadowStaticIndexCount, 0, 0);
                            renderProfile.Mark(L"ShadowStatic");
                        }
                    }
                }
