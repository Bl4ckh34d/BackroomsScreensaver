    void UploadSceneConstants(const SceneConstants& cb) {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (SUCCEEDED(d3dRuntime_.context->Map(renderBuffers_.constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            std::memcpy(mapped.pData, &cb, sizeof(cb));
            d3dRuntime_.context->Unmap(renderBuffers_.constantBuffer.Get(), 0);
        }
    }
