// Main opaque, floor/ceiling, dynamic, water, and transparent scene passes.

        ID3D11ShaderResourceView* srvs[] = {
            materialTextures_.albedoSrv.Get(),
            materialTextures_.normalSrv.Get(),
            shadowResources_.shadowSrv.Get(),
            runtimeTextures_.mazeSrv.Get(),
            materialTextures_.materialPropsSrv.Get(),
            runtimeTextures_.flashlightPatternSrv.Get(),
            runtimeTextures_.lampDamageSrv.Get(),
            nullptr,
            nullptr,
            runtimeTextures_.loosePagesSrv.Get(),
            shadowResources_.fixtureShadowSrv.Get(),
            materialTextures_.ceilingAlbedoSrv.Get(),
            materialTextures_.ceilingNormalSrv.Get(),
            materialTextures_.ceilingPropsSrv.Get(),
            runtimeTextures_.customMenuSrv.Get(),
            materialTextures_.doorAlbedoSrv.Get(),
            materialTextures_.doorNormalSrv.Get(),
            materialTextures_.doorPropsSrv.Get(),
            materialTextures_.doorFrameAlbedoSrv.Get(),
            materialTextures_.doorFrameNormalSrv.Get(),
            materialTextures_.doorFramePropsSrv.Get()
        };
        ID3D11SamplerState* samplers[] = {pipelineStates_.sampler.Get(), pipelineStates_.shadowSampler.Get()};
        d3dRuntime_.context->OMSetRenderTargets(1, &sceneTarget, renderTargetRuntime_.dsv.Get());
        d3dRuntime_.context->RSSetViewports(1, &vp);
        d3dRuntime_.context->IASetInputLayout(inputLayouts_.inputLayout.Get());
        d3dRuntime_.context->IASetPrimitiveTopology(useFleshTessellation
            ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
            : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3dRuntime_.context->VSSetShader(shaders_.vertexShader.Get(), nullptr, 0);
        d3dRuntime_.context->HSSetShader(useFleshTessellation ? shaders_.hullShader.Get() : nullptr, nullptr, 0);
        d3dRuntime_.context->DSSetShader(useFleshTessellation ? shaders_.domainShader.Get() : nullptr, nullptr, 0);
        d3dRuntime_.context->PSSetShader(shaders_.pixelShader.Get(), nullptr, 0);
        d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
        d3dRuntime_.context->HSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
        d3dRuntime_.context->DSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
        d3dRuntime_.context->PSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
        d3dRuntime_.context->PSSetShaderResources(0, static_cast<UINT>(std::size(srvs)), srvs);
        if (useFleshTessellation) {
            d3dRuntime_.context->DSSetShaderResources(0, static_cast<UINT>(std::size(srvs)), srvs);
            d3dRuntime_.context->DSSetSamplers(0, 1, pipelineStates_.sampler.GetAddressOf());
        }
        d3dRuntime_.context->PSSetSamplers(0, 2, samplers);
        d3dRuntime_.context->RSSetState(pipelineStates_.rasterState.Get());
        d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthState.Get(), 0);

        auto bindStaticScenePipeline = [&]() {
            d3dRuntime_.context->IASetInputLayout(inputLayouts_.inputLayout.Get());
            d3dRuntime_.context->IASetPrimitiveTopology(useFleshTessellation
                ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
                : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            d3dRuntime_.context->IASetVertexBuffers(0, 1, renderBuffers_.vertexBuffer.GetAddressOf(), &stride, &offset);
            d3dRuntime_.context->IASetIndexBuffer(renderBuffers_.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            d3dRuntime_.context->VSSetShader(shaders_.vertexShader.Get(), nullptr, 0);
            d3dRuntime_.context->HSSetShader(useFleshTessellation ? shaders_.hullShader.Get() : nullptr, nullptr, 0);
            d3dRuntime_.context->DSSetShader(useFleshTessellation ? shaders_.domainShader.Get() : nullptr, nullptr, 0);
            d3dRuntime_.context->PSSetShader(shaders_.pixelShader.Get(), nullptr, 0);
            d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->HSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->DSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->PSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->PSSetShaderResources(0, 14, srvs);
            if (useFleshTessellation) {
                d3dRuntime_.context->DSSetShaderResources(0, 14, srvs);
                d3dRuntime_.context->DSSetSamplers(0, 1, pipelineStates_.sampler.GetAddressOf());
            }
            d3dRuntime_.context->PSSetSamplers(0, 2, samplers);
        };

        auto bindInstancedScenePipeline = [&]() {
            ID3D11Buffer* buffers[] = {renderBuffers_.instancedVertexBuffer.Get(), renderBuffers_.instancedInstanceBuffer.Get()};
            UINT strides[] = {sizeof(Vertex), sizeof(StaticInstanceData)};
            UINT offsets[] = {0, 0};
            d3dRuntime_.context->IASetInputLayout(inputLayouts_.instancedInputLayout.Get());
            d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            d3dRuntime_.context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
            d3dRuntime_.context->IASetIndexBuffer(renderBuffers_.instancedIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            d3dRuntime_.context->VSSetShader(shaders_.instancedVertexShader.Get(), nullptr, 0);
            d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
            d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
            d3dRuntime_.context->PSSetShader(shaders_.pixelShader.Get(), nullptr, 0);
            d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->PSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->PSSetShaderResources(0, 14, srvs);
            d3dRuntime_.context->PSSetSamplers(0, 2, samplers);
        };

        auto bindDynamicScenePipeline = [&]() {
            d3dRuntime_.context->IASetInputLayout(inputLayouts_.inputLayout.Get());
            d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
            d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
            d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            d3dRuntime_.context->IASetVertexBuffers(0, 1, renderBuffers_.dynamicBuffer.GetAddressOf(), &stride, &offset);
            d3dRuntime_.context->VSSetShader(shaders_.vertexShader.Get(), nullptr, 0);
            d3dRuntime_.context->PSSetShader(shaders_.pixelShader.Get(), nullptr, 0);
            d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->PSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
            d3dRuntime_.context->PSSetShaderResources(0, 14, srvs);
            d3dRuntime_.context->PSSetSamplers(0, 2, samplers);
        };
