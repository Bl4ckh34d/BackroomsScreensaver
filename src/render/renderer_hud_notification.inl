// HUD notification texture and textured overlay drawing helpers. 
// Included inside Renderer's private section from renderer_overlays.inl.

    bool UpdateHudNotificationTexture() {
        if (!hudNotification_.textureDirty) return hudNotification_.srv != nullptr;
        hudNotification_.textureDirty = false;
        hudNotification_.texture.Reset();
        hudNotification_.srv.Reset();
        if (hudNotification_.text.empty() || !d3dRuntime_.device) return false;

        HDC dc = CreateCompatibleDC(nullptr);
        if (!dc) {
            if (dc) DeleteDC(dc);
            return false;
        }
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));
        const int fontPx = 34;
        HFONT font = CreateFontW(-fontPx, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(dc, font);

        constexpr int paddingX = 34;
        constexpr int paddingY = 16;
        constexpr int maxTextW = 760;
        RECT measure{0, 0, maxTextW, 0};
        DrawTextW(dc, hudNotification_.text.c_str(), -1, &measure,
            DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
        bool wrap = (measure.right - measure.left) > maxTextW || hudNotification_.text.find(L'\n') != std::wstring::npos;
        if (wrap) {
            measure = {0, 0, maxTextW, 0};
            DrawTextW(dc, hudNotification_.text.c_str(), -1, &measure,
                DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
        }
        int measuredW = static_cast<int>(measure.right - measure.left);
        int measuredH = static_cast<int>(measure.bottom - measure.top);
        int texW = std::clamp(measuredW + paddingX * 2, 220, maxTextW + paddingX * 2);
        int texH = std::clamp(measuredH + paddingY * 2, 58, 160);
        hudNotification_.textureWidth = texW;
        hudNotification_.textureHeight = texH;

        std::vector<uint8_t> pixels(static_cast<size_t>(texW) * static_cast<size_t>(texH) * 4, 0);
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = texW;
        bmi.bmiHeader.biHeight = -texH;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!bmp || !bits) {
            SelectObject(dc, oldFont);
            DeleteObject(font);
            DeleteDC(dc);
            return false;
        }
        HGDIOBJ oldBmp = SelectObject(dc, bmp);
        std::memset(bits, 0, pixels.size());

        RECT textRect{paddingX, paddingY / 2, texW - paddingX, texH - paddingY / 2};
        UINT textFlags = wrap
            ? (DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS)
            : (DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        DrawTextW(dc, hudNotification_.text.c_str(), -1, &textRect, textFlags);
        SelectObject(dc, oldFont);
        DeleteObject(font);
        SelectObject(dc, oldBmp);

        const uint8_t* src = static_cast<const uint8_t*>(bits);
        for (int y = 0; y < texH; ++y) {
            for (int x = 0; x < texW; ++x) {
                size_t i = static_cast<size_t>((y * texW + x) * 4);
                uint8_t a = std::max(src[i + 0], std::max(src[i + 1], src[i + 2]));
                if (a == 0) continue;
                pixels[i + 0] = 218;
                pixels[i + 1] = 230;
                pixels[i + 2] = 246;
                pixels[i + 3] = a;
            }
        }
        DeleteObject(bmp);
        DeleteDC(dc);

        D3D11_TEXTURE2D_DESC td{};
        td.Width = texW;
        td.Height = texH;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA init{pixels.data(), static_cast<UINT>(texW * 4), 0};
        if (FAILED(d3dRuntime_.device->CreateTexture2D(&td, &init, &hudNotification_.texture))) return false;
        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MipLevels = 1;
        return SUCCEEDED(d3dRuntime_.device->CreateShaderResourceView(hudNotification_.texture.Get(), &sd, &hudNotification_.srv));
    }

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
