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
