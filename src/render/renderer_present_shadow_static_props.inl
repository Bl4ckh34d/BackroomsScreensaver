                if (staticSceneGeometry_.propShadowIndexCount > 0) {
                    d3dRuntime_.context->PSSetShader(nullptr, nullptr, 0);
                    StartupProfileLine(staticSceneGeometry_.propShadowChunks.empty()
                        ? L"Render before StaticPropShadow DrawIndexed"
                        : L"Render before StaticPropShadow chunked DrawIndexed");
                    UINT drawn = 0;
                    if (!staticSceneGeometry_.propShadowChunks.empty()) {
                        drawn = drawVisibleChunks(staticSceneGeometry_.propShadowChunks, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                            shadowTileAverage * 1.4f, false, 1);
                    }
                    if (staticSceneGeometry_.propShadowChunks.empty()) {
                        d3dRuntime_.context->DrawIndexed(staticSceneGeometry_.propShadowIndexCount, staticSceneGeometry_.propShadowStartIndex, 0);
                    }
                    renderProfile.Mark(L"StaticPropShadow");
                    d3dRuntime_.context->PSSetShader(shaders_.shadowPixelShader.Get(), nullptr, 0);
                }
