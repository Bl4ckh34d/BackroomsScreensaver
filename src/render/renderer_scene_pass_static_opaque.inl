        if (!monsterPreview_.active) {
            bindStaticScenePipeline();
            d3dRuntime_.context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            UINT opaqueIndexCount = std::min(staticSceneGeometry_.floorCeilingStartIndex, staticSceneGeometry_.indexCount);
            if (opaqueIndexCount > 0) {
                StartupProfileLine(staticSceneGeometry_.opaqueChunks.empty()
                    ? L"Render before MainOpaque DrawIndexed"
                    : L"Render before MainOpaque chunked DrawIndexed");
                UINT drawn = 0;
                if (!staticSceneGeometry_.opaqueChunks.empty()) {
                    drawn = drawVisibleChunks(staticSceneGeometry_.opaqueChunks, eyePos, viewDirFloat, mainCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 2);
                    if (StartupProfileEnabled()) {
                        std::wstringstream counts;
                        counts << L"MainOpaque visible indices=" << drawn << L"/" << opaqueIndexCount;
                        StartupProfileLine(counts.str());
                    }
                }
                if (staticSceneGeometry_.opaqueChunks.empty()) {
                    d3dRuntime_.context->DrawIndexed(opaqueIndexCount, 0, 0);
                }
                if (!staticSceneGeometry_.instancedOpaqueChunks.empty() && renderBuffers_.instancedVertexBuffer && renderBuffers_.instancedIndexBuffer && renderBuffers_.instancedInstanceBuffer) {
                    bindInstancedScenePipeline();
                    StartupProfileLine(L"Render before MainOpaque instanced DrawIndexedInstanced");
                    UINT instancedDrawn = drawVisibleInstancedChunks(staticSceneGeometry_.instancedOpaqueChunks, eyePos, viewDirFloat, mainCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 3);
                    if (StartupProfileEnabled()) {
                        std::wstringstream counts;
                        counts << L"MainOpaque visible instanced indices=" << instancedDrawn
                            << L"/" << (static_cast<uint64_t>(staticSceneGeometry_.instancedIndexCount) * static_cast<uint64_t>(staticSceneGeometry_.instancedInstanceCount));
                        StartupProfileLine(counts.str());
                    }
                    bindStaticScenePipeline();
                }
                renderProfile.Mark(L"MainOpaque");
            }
            MarkGpuProfile(GpuProfileMarker::MainOpaque);
            if (staticSceneGeometry_.floorCeilingIndexCount > 0) {
                StartupProfileLine(staticSceneGeometry_.floorCeilingChunks.empty()
                    ? L"Render before FloorCeiling DrawIndexed"
                    : L"Render before FloorCeiling chunked DrawIndexed");
                UINT drawn = 0;
                if (!staticSceneGeometry_.floorCeilingChunks.empty()) {
                    drawn = drawVisibleChunks(staticSceneGeometry_.floorCeilingChunks, eyePos, viewDirFloat, mainCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 2);
                    if (StartupProfileEnabled()) {
                        std::wstringstream counts;
                        counts << L"FloorCeiling visible indices=" << drawn << L"/" << staticSceneGeometry_.floorCeilingIndexCount;
                        StartupProfileLine(counts.str());
                    }
                }
                if (staticSceneGeometry_.floorCeilingChunks.empty()) {
                    d3dRuntime_.context->DrawIndexed(staticSceneGeometry_.floorCeilingIndexCount, staticSceneGeometry_.floorCeilingStartIndex, 0);
                }
                renderProfile.Mark(L"FloorCeiling");
            }
        } else {
            MarkGpuProfile(GpuProfileMarker::MainOpaque);
        }
        MarkGpuProfile(GpuProfileMarker::FloorCeiling);
