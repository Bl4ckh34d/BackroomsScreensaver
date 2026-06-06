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
        renderTargetRuntime_.sceneColorMsaaRtv.Reset();
        renderTargetRuntime_.sceneColorMsaa.Reset();
        renderTargetRuntime_.sceneColorRtv.Reset();
        renderTargetRuntime_.sceneColor.Reset();
        renderTargetRuntime_.sceneWidth = 0;
        renderTargetRuntime_.sceneHeight = 0;
        renderTargetRuntime_.sceneSampleCount = 1;
        HRESULT hr = d3dRuntime_.swapChain->ResizeBuffers(0, static_cast<UINT>(w), static_cast<UINT>(h), DXGI_FORMAT_UNKNOWN, 0);
        if (SUCCEEDED(hr)) CreateBackBuffer();
    }
