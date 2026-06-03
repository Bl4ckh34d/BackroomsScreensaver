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
