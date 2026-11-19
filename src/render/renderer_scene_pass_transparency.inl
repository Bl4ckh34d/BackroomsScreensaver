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
                auto* context = d3dRuntime_.context.Get();
                const auto& liquid = renderTargetRuntime_.liquid;
                const bool resolveLiquid = postAvailable && liquid.resolveConstants && shaders_.liquidResolvePixelShader;
                auto drawLiquids = [&]() {
                    if (!staticSceneGeometry_.waterChunks.empty()) {
                        drawVisibleChunks(staticSceneGeometry_.waterChunks, eyePos, viewDirFloat, transparentCullDistance, mainConeCos,
                            mainForceVisibleDistance, true, 4);
                    } else {
                        context->DrawIndexed(staticSceneGeometry_.waterIndexCount, staticSceneGeometry_.waterStartIndex, 0);
                    }
                };
                context->OMSetDepthStencilState(pipelineStates_.liquidDepthStencilState.Get(), 0);
                context->PSSetShader(shaders_.liquidPixelShader.Get(), nullptr, 0);
                StartupProfileLine(L"Render before StaticWater DrawIndexed");
                if (resolveLiquid) {
                    auto liquidViewport = vp;
                    liquidViewport.Width = static_cast<float>(liquid.width);
                    liquidViewport.Height = static_cast<float>(liquid.height);
                    const float clearLiquid[4]{};
                    context->ClearRenderTargetView(liquid.color.Get(), clearLiquid);
                    context->ClearDepthStencilView(liquid.depth.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
                    context->OMSetRenderTargets(1, liquid.color.GetAddressOf(), liquid.depth.Get());
                    context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
                    context->RSSetViewports(1, &liquidViewport);
                    drawLiquids();
                    context->OMSetRenderTargets(1, &sceneTarget, renderTargetRuntime_.dsv.Get());
                    context->RSSetViewports(1, &vp);
                    ID3D11ShaderResourceView* liquidInputs[]{liquid.colorSrv.Get(), liquid.depthSrv.Get()};
                    context->PSSetShaderResources(23, 2, liquidInputs);
                    context->PSSetConstantBuffers(1, 1, liquid.resolveConstants.GetAddressOf());
                    context->PSSetShader(shaders_.liquidResolvePixelShader.Get(), nullptr, 0);
                }
                context->OMSetBlendState(pipelineStates_.alphaBlend.Get(), blendFactor, 0xffffffff);
                drawLiquids();
                if (resolveLiquid) {
                    ID3D11ShaderResourceView* unbound[2]{};
                    context->PSSetShaderResources(23, 2, unbound);
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
