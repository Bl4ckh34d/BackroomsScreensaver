    void DrawHudNotificationText(float x, float y, float w, float h, float alpha) {
        if (alpha <= 0.002f || !UpdateHudNotificationTexture() || !hudNotification_.srv ||
            !renderBuffers_.overlayBuffer || !shaders_.texturedOverlayVertexShader || !shaders_.texturedOverlayPixelShader ||
            !inputLayouts_.texturedOverlayInputLayout || !pipelineStates_.postSampler) {
            return;
        }
        auto ndcX = [&](float px) { return px / static_cast<float>(hostRuntime_.width) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(hostRuntime_.height) * 2.0f; };
        TexturedOverlayVertex verts[6] = {
            {{ndcX(x), ndcY(y + h)}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x), ndcY(y)}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x + w), ndcY(y)}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x), ndcY(y + h)}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x + w), ndcY(y)}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x + w), ndcY(y + h)}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, alpha}}
        };

        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(d3dRuntime_.context->Map(renderBuffers_.overlayBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;
        std::memcpy(mapped.pData, verts, sizeof(verts));
        d3dRuntime_.context->Unmap(renderBuffers_.overlayBuffer.Get(), 0);

        UINT stride = sizeof(TexturedOverlayVertex);
        UINT offset = 0;
        float blendFactor[4] = {};
        d3dRuntime_.context->IASetInputLayout(inputLayouts_.texturedOverlayInputLayout.Get());
        d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3dRuntime_.context->IASetVertexBuffers(0, 1, renderBuffers_.overlayBuffer.GetAddressOf(), &stride, &offset);
        d3dRuntime_.context->VSSetShader(shaders_.texturedOverlayVertexShader.Get(), nullptr, 0);
        d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->PSSetShader(shaders_.texturedOverlayPixelShader.Get(), nullptr, 0);
        d3dRuntime_.context->OMSetDepthStencilState(pipelineStates_.depthDisabledState.Get(), 0);
        d3dRuntime_.context->OMSetBlendState(pipelineStates_.alphaBlend.Get(), blendFactor, 0xffffffff);
        ID3D11ShaderResourceView* srv = hudNotification_.srv.Get();
        ID3D11SamplerState* sampler = pipelineStates_.postSampler.Get();
        d3dRuntime_.context->PSSetShaderResources(0, 1, &srv);
        d3dRuntime_.context->PSSetSamplers(0, 1, &sampler);
        d3dRuntime_.context->Draw(6, 0);
        ID3D11ShaderResourceView* nullSrv = nullptr;
        d3dRuntime_.context->PSSetShaderResources(0, 1, &nullSrv);
    }
