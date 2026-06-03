        if (dynamicGeometry_.opaqueVertexCount > 0 || dynamicGeometry_.transparentVertexCount > 0) {
            bindDynamicScenePipeline();
        }
        if (dynamicGeometry_.opaqueVertexCount > 0) {
            d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthState.Get(), 0);
            d3dRuntime_.context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            StartupProfileLine(L"Render before DynamicOpaque Draw");
            d3dRuntime_.context->Draw(dynamicGeometry_.opaqueVertexCount, 0);
            renderProfile.Mark(L"DynamicOpaque");
        }
        MarkGpuProfile(GpuProfileMarker::DynamicOpaque);
        if (!monsterPreview_.active) {
            if (staticSceneGeometry_.waterIndexCount > 0 || staticSceneGeometry_.transparentIndexCount > 0) {
                bindStaticScenePipeline();
            }
            if (staticSceneGeometry_.waterIndexCount > 0) {
                d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.liquidDepthStencilState.Get(), 0);
                d3dRuntime_.context->OMSetBlendState(pipelineStates_.alphaBlend.Get(), blendFactor, 0xffffffff);
                d3dRuntime_.context->PSSetShader(shaders_.liquidPixelShader ? shaders_.liquidPixelShader.Get() : shaders_.pixelShader.Get(), nullptr, 0);
                StartupProfileLine(L"Render before StaticWater DrawIndexed");
                if (!staticSceneGeometry_.waterChunks.empty()) {
                    drawVisibleChunks(staticSceneGeometry_.waterChunks, eyePos, viewDirFloat, transparentCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 4);
                } else {
                    d3dRuntime_.context->DrawIndexed(staticSceneGeometry_.waterIndexCount, staticSceneGeometry_.waterStartIndex, 0);
                }
                renderProfile.Mark(L"StaticWater");
                d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthState.Get(), 0);
                d3dRuntime_.context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            }
            MarkGpuProfile(GpuProfileMarker::StaticWater);
            if (staticSceneGeometry_.transparentIndexCount > 0) {
                d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthReadOnlyState.Get(), 0);
                d3dRuntime_.context->OMSetBlendState(pipelineStates_.alphaBlend.Get(), blendFactor, 0xffffffff);
                d3dRuntime_.context->PSSetShader(shaders_.pixelShader.Get(), nullptr, 0);
                StartupProfileLine(L"Render before StaticTransparent DrawIndexed");
                if (!staticSceneGeometry_.transparentChunks.empty()) {
                    drawVisibleChunks(staticSceneGeometry_.transparentChunks, eyePos, viewDirFloat, transparentCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 4);
                } else {
                    d3dRuntime_.context->DrawIndexed(staticSceneGeometry_.transparentIndexCount, staticSceneGeometry_.transparentStartIndex, 0);
                }
                renderProfile.Mark(L"StaticTransparent");
                d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthState.Get(), 0);
                d3dRuntime_.context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            }
        } else {
            MarkGpuProfile(GpuProfileMarker::StaticWater);
        }
        MarkGpuProfile(GpuProfileMarker::StaticTransparent);
        if (dynamicGeometry_.transparentVertexCount > 0) {
            bindDynamicScenePipeline();
            d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthReadOnlyState.Get(), 0);
            d3dRuntime_.context->OMSetBlendState(pipelineStates_.alphaBlend.Get(), blendFactor, 0xffffffff);
            StartupProfileLine(L"Render before DynamicTransparent Draw");
            d3dRuntime_.context->Draw(dynamicGeometry_.transparentVertexCount, dynamicGeometry_.opaqueVertexCount);
            renderProfile.Mark(L"DynamicTransparent");
        }
        MarkGpuProfile(GpuProfileMarker::DynamicTransparent);
