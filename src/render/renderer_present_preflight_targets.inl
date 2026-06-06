// Render preflight, target clear, viewport, camera, projection, and light-space setup.

        presentRuntime_.lastCompleted = false;
        if (!presentRuntime_.enabled) {
            presentRuntime_.lastCompleted = true;
            return;
        }
        if (!renderTargetRuntime_.rtv || !renderTargetRuntime_.dsv) {
            presentRuntime_.lastCompleted = true;
            return;
        }
        StartupProfile renderProfile(L"Render");
        BeginGpuProfileFrame();
        bool postAvailable = renderTargetRuntime_.sceneColorRtv && renderTargetRuntime_.sceneColorSrv && shaders_.postVertexShader && shaders_.postPixelShader && pipelineStates_.postSampler;
        ID3D11RenderTargetView* sceneTarget = postAvailable
            ? (renderTargetRuntime_.sceneColorMsaaRtv ? renderTargetRuntime_.sceneColorMsaaRtv.Get() : renderTargetRuntime_.sceneColorRtv.Get())
            : renderTargetRuntime_.rtv.Get();

        float clear[4] = {0.004f, 0.004f, 0.004f, 1.0f};
        if (monsterPreview_.active) {
            if (monsterPreview_.view == MonsterPreviewView::Orbit) {
                clear[0] = clear[1] = clear[2] = 0.93f;
            } else {
                clear[0] = clear[1] = clear[2] = 0.0f;
            }
        }
        d3dRuntime_.context->ClearRenderTargetView(renderTargetRuntime_.rtv.Get(), clear);
        if (postAvailable && renderTargetRuntime_.sceneColorMsaaRtv) {
            d3dRuntime_.context->ClearRenderTargetView(renderTargetRuntime_.sceneColorMsaaRtv.Get(), clear);
        } else if (postAvailable) {
            d3dRuntime_.context->ClearRenderTargetView(renderTargetRuntime_.sceneColorRtv.Get(), clear);
        }
        d3dRuntime_.context->ClearDepthStencilView(renderTargetRuntime_.dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        renderProfile.Mark(L"ClearTargets");
        MarkGpuProfile(GpuProfileMarker::ClearTargets);

        D3D11_VIEWPORT vp{};
        vp.Width = postAvailable && renderTargetRuntime_.sceneWidth > 0
            ? static_cast<float>(renderTargetRuntime_.sceneWidth)
            : static_cast<float>(hostRuntime_.width);
        vp.Height = postAvailable && renderTargetRuntime_.sceneHeight > 0
            ? static_cast<float>(renderTargetRuntime_.sceneHeight)
            : static_cast<float>(hostRuntime_.height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        d3dRuntime_.context->RSSetViewports(1, &vp);
