// Full-screen postprocess overlay draw helper. 
// Included inside Renderer's private section from renderer_overlays.inl.

    void DrawPostProcess() {
        if (!renderTargetRuntime_.sceneColorSrv || !shaders_.postVertexShader || !shaders_.postPixelShader || !pipelineStates_.postSampler || !renderTargetRuntime_.rtv) return;
        float blendFactor[4] = {};
        ID3D11RenderTargetView* target = renderTargetRuntime_.rtv.Get();
        d3dRuntime_.context->OMSetRenderTargets(1, &target, nullptr);
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
