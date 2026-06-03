// Flashlight and fixture depth shadow passes.

        auto renderDepthShadow = [&](ID3D11DepthStencilView* shadowDsv, UINT shadowSize, const XMMATRIX& shadowViewProj,
                                     XMFLOAT3 shadowOrigin, XMFLOAT3 shadowDirection, float shadowRange, float shadowConeCos) {
            if (!shadowDsv) return;
            ID3D11ShaderResourceView* nullSrvs[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
            d3dRuntime_.context->PSSetShaderResources(0, 14, nullSrvs);
            d3dRuntime_.context->DSSetShaderResources(0, 14, nullSrvs);
            d3dRuntime_.context->ClearDepthStencilView(shadowDsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
            d3dRuntime_.context->OMSetRenderTargets(0, nullptr, shadowDsv);

            D3D11_VIEWPORT shadowVp{};
            shadowVp.Width = static_cast<float>(shadowSize);
            shadowVp.Height = static_cast<float>(shadowSize);
            shadowVp.MinDepth = 0.0f;
            shadowVp.MaxDepth = 1.0f;
            d3dRuntime_.context->RSSetViewports(1, &shadowVp);

            SceneConstants shadowCb = cb;
            XMStoreFloat4x4(&shadowCb.viewProj, shadowViewProj);
            UploadSceneConstants(shadowCb);

            d3dRuntime_.context->IASetInputLayout(inputLayouts_.inputLayout.Get());
            d3dRuntime_.context->IASetPrimitiveTopology(useFleshTessellation
                ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
                : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            d3dRuntime_.context->IASetVertexBuffers(0, 1, renderBuffers_.vertexBuffer.GetAddressOf(), &stride, &offset);
            d3dRuntime_.context->IASetIndexBuffer(renderBuffers_.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            d3dRuntime_.context->VSSetShader(shaders_.vertexShader.Get(), nullptr, 0);
            d3dRuntime_.context->HSSetShader(useFleshTessellation ? shaders_.hullShader.Get() : nullptr, nullptr, 0);
            d3dRuntime_.context->DSSetShader(useFleshTessellation ? shaders_.domainShader.Get() : nullptr, nullptr, 0);
            d3dRuntime_.context->PSSetShader(shaders_.shadowPixelShader.Get(), nullptr, 0);
            d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->HSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->DSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            if (useFleshTessellation) {
                ID3D11ShaderResourceView* dsSrvs[] = {nullptr, materialTextures_.normalSrv.Get(), nullptr, nullptr, nullptr, nullptr, nullptr};
                d3dRuntime_.context->DSSetShaderResources(0, 7, dsSrvs);
                d3dRuntime_.context->DSSetSamplers(0, 1, pipelineStates_.sampler.GetAddressOf());
            }
            d3dRuntime_.context->RSSetState(pipelineStates_.shadowRasterState.Get());
            d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthState.Get(), 0);
            d3dRuntime_.context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            if (!monsterPreview_.active) {
                float shadowTileAverage = visibilityMaze ? visibilityMaze->TileAverage() : 1.0f;
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
                if (!staticSceneGeometry_.instancedPropShadowChunks.empty() && renderBuffers_.instancedVertexBuffer && renderBuffers_.instancedIndexBuffer && renderBuffers_.instancedInstanceBuffer) {
                    ID3D11Buffer* instBuffers[] = {renderBuffers_.instancedVertexBuffer.Get(), renderBuffers_.instancedInstanceBuffer.Get()};
                    UINT instStrides[] = {sizeof(Vertex), sizeof(StaticInstanceData)};
                    UINT instOffsets[] = {0, 0};
                    d3dRuntime_.context->IASetInputLayout(inputLayouts_.instancedInputLayout.Get());
                    d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    d3dRuntime_.context->IASetVertexBuffers(0, 2, instBuffers, instStrides, instOffsets);
                    d3dRuntime_.context->IASetIndexBuffer(renderBuffers_.instancedIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                    d3dRuntime_.context->VSSetShader(shaders_.instancedVertexShader.Get(), nullptr, 0);
                    d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
                    d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
                    d3dRuntime_.context->PSSetShader(nullptr, nullptr, 0);
                    d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
                    StartupProfileLine(L"Render before InstancedPropShadow DrawIndexedInstanced");
                    drawVisibleInstancedChunks(staticSceneGeometry_.instancedPropShadowChunks, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                        shadowTileAverage * 1.4f, false, 1);
                    renderProfile.Mark(L"InstancedPropShadow");
                    d3dRuntime_.context->PSSetShader(shaders_.shadowPixelShader.Get(), nullptr, 0);
                }
            }

            if (dynamicGeometry_.opaqueVertexCount > 0 && sessionRuntime_.mode != RendererRuntimeMode::MainMenu) {
                d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
                d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
                d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                d3dRuntime_.context->IASetVertexBuffers(0, 1, renderBuffers_.dynamicBuffer.GetAddressOf(), &stride, &offset);
                StartupProfileLine(L"Render before ShadowDynamic Draw");
                d3dRuntime_.context->Draw(dynamicGeometry_.opaqueVertexCount, 0);
                renderProfile.Mark(L"ShadowDynamic");
            }
        };

        if (shadowResources_.shadowDsv && settingsRuntime_.live.flashlightShadows && settingsRuntime_.live.flashlightShadowStrength > 0.001f &&
            flashlightIntensity > 0.001f && transitionFade < 0.995f) {
            renderProfile.Mark(L"BeginShadowPass");
            renderDepthShadow(shadowResources_.shadowDsv.Get(), shadowResources_.shadowMapSize, lightViewProj,
                lightPosFloat, lightDirFloat, shadowDistance, std::cos(shadowFov * 0.5f));
        }
        MarkGpuProfile(GpuProfileMarker::FlashlightShadow);
        if (fixtureShadowActive && settingsRuntime_.live.flashlightShadowStrength > 0.001f && transitionFade < 0.995f) {
            renderProfile.Mark(L"BeginFixtureShadow");
            renderDepthShadow(shadowResources_.fixtureShadowDsv.Get(), shadowResources_.fixtureShadowMapSize, fixtureLightViewProj,
                fixtureShadowPos, fixtureShadowDir, fixtureShadowRange, std::cos(116.0f * 0.5f * kPi / 180.0f));
        }
        MarkGpuProfile(GpuProfileMarker::FixtureShadow);
        UploadSceneConstants(cb);
        renderProfile.Mark(L"UploadSceneConstants");
        UploadLampDamageTexture();
        renderProfile.Mark(L"UploadLampDamageTexture");
        UpdateCustomMenuTexture();
        MarkGpuProfile(GpuProfileMarker::Uploads);

