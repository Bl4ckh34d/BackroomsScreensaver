// Shared overlay draw submission helper. 
// Included inside Renderer's private section from renderer_overlays.inl.

    void DrawOverlayVertices(const std::vector<OverlayVertex>& verts) {
        if (verts.empty() || !renderBuffers_.overlayBuffer || !shaders_.overlayVertexShader || !shaders_.overlayPixelShader) return;
        size_t count = std::min(verts.size(), static_cast<size_t>(kOverlayVertexCapacity));
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(d3dRuntime_.context->Map(renderBuffers_.overlayBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;
        std::memcpy(mapped.pData, verts.data(), count * sizeof(OverlayVertex));
        d3dRuntime_.context->Unmap(renderBuffers_.overlayBuffer.Get(), 0);

        UINT stride = sizeof(OverlayVertex);
        UINT offset = 0;
        float blendFactor[4] = {};
        d3dRuntime_.context->IASetInputLayout(inputLayouts_.overlayInputLayout.Get());
        d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3dRuntime_.context->IASetVertexBuffers(0, 1, renderBuffers_.overlayBuffer.GetAddressOf(), &stride, &offset);
        d3dRuntime_.context->VSSetShader(shaders_.overlayVertexShader.Get(), nullptr, 0);
        d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->PSSetShader(shaders_.overlayPixelShader.Get(), nullptr, 0);
        d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthDisabledState.Get(), 0);
        d3dRuntime_.context->OMSetBlendState(pipelineStates_.alphaBlend.Get(), blendFactor, 0xffffffff);
        d3dRuntime_.context->Draw(static_cast<UINT>(count), 0);
    }
