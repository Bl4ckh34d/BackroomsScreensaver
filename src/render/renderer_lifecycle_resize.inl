    // Renderer resize, frame tick, and present controls.

    void Resize(int w, int h) {
        if (!d3dRuntime_.device || w <= 0 || h <= 0) return;
        hostRuntime_.width = w;
        hostRuntime_.height = h;
        d3dRuntime_.context->OMSetRenderTargets(0, nullptr, nullptr);
        renderTargetRuntime_.rtv.Reset();
        renderTargetRuntime_.dsv.Reset();
        renderTargetRuntime_.depth.Reset();
        renderTargetRuntime_.sceneColorSrv.Reset();
        renderTargetRuntime_.sceneColorRtv.Reset();
        renderTargetRuntime_.sceneColor.Reset();
        HRESULT hr = d3dRuntime_.swapChain->ResizeBuffers(0, static_cast<UINT>(w), static_cast<UINT>(h), DXGI_FORMAT_UNKNOWN, 0);
        if (SUCCEEDED(hr)) CreateBackBuffer();
    }
