// Full-screen postprocess overlay draw helper. 
// Included inside Renderer's private section from renderer_overlays.inl.

    void DrawPostProcess() {
        if (!renderTargetRuntime_.sceneColorSrv || !shaders_.postVertexShader || !shaders_.postPixelShader || !pipelineStates_.postSampler || !renderTargetRuntime_.rtv) return;
        float blendFactor[4] = {};
        ID3D11RenderTargetView* target = renderTargetRuntime_.rtv.Get();
        d3dRuntime_.context->OMSetRenderTargets(1, &target, nullptr);
        if (renderTargetRuntime_.sceneColorMsaa && renderTargetRuntime_.sceneColor && renderTargetRuntime_.sceneSampleCount > 1) {
            d3dRuntime_.context->ResolveSubresource(renderTargetRuntime_.sceneColor.Get(), 0,
                renderTargetRuntime_.sceneColorMsaa.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
        }
        D3D11_VIEWPORT fullVp{};
        fullVp.Width = static_cast<float>(hostRuntime_.width);
        fullVp.Height = static_cast<float>(hostRuntime_.height);
        fullVp.MinDepth = 0.0f;
        fullVp.MaxDepth = 1.0f;
        d3dRuntime_.context->RSSetViewports(1, &fullVp);
        d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthDisabledState.Get(), 0);
        d3dRuntime_.context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
        d3dRuntime_.context->RSSetState(pipelineStates_.rasterState.Get());
        d3dRuntime_.context->IASetInputLayout(nullptr);
        d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3dRuntime_.context->VSSetShader(shaders_.postVertexShader.Get(), nullptr, 0);
        d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->PSSetShader(shaders_.postPixelShader.Get(), nullptr, 0);
        d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
        d3dRuntime_.context->PSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
        ID3D11ShaderResourceView* srv = renderTargetRuntime_.sceneColorSrv.Get();
        ID3D11SamplerState* sampler = pipelineStates_.postSampler.Get();
        d3dRuntime_.context->PSSetShaderResources(0, 1, &srv);
        d3dRuntime_.context->PSSetSamplers(0, 1, &sampler);
        d3dRuntime_.context->Draw(3, 0);
        ID3D11ShaderResourceView* nullSrv = nullptr;
        d3dRuntime_.context->PSSetShaderResources(0, 1, &nullSrv);
    }
