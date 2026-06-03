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
