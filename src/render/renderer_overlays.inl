    void DrawOverlayVertices(const std::vector<OverlayVertex>& verts) {
        if (verts.empty() || !overlayBuffer_ || !overlayVertexShader_ || !overlayPixelShader_) return;
        size_t count = std::min(verts.size(), static_cast<size_t>(kOverlayVertexCapacity));
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context_->Map(overlayBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;
        std::memcpy(mapped.pData, verts.data(), count * sizeof(OverlayVertex));
        context_->Unmap(overlayBuffer_.Get(), 0);

        UINT stride = sizeof(OverlayVertex);
        UINT offset = 0;
        float blendFactor[4] = {};
        context_->IASetInputLayout(overlayInputLayout_.Get());
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->IASetVertexBuffers(0, 1, overlayBuffer_.GetAddressOf(), &stride, &offset);
        context_->VSSetShader(overlayVertexShader_.Get(), nullptr, 0);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        context_->PSSetShader(overlayPixelShader_.Get(), nullptr, 0);
        context_->OMSetDepthStencilState(depthDisabledState_.Get(), 0);
        context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
        context_->Draw(static_cast<UINT>(count), 0);
    }

    bool UpdateHudNotificationTexture() {
        if (!hudNotificationTextureDirty_) return hudNotificationSrv_ != nullptr;
        hudNotificationTextureDirty_ = false;
        hudNotificationTexture_.Reset();
        hudNotificationSrv_.Reset();
        if (hudNotificationText_.empty() || !device_) return false;

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
        DrawTextW(dc, hudNotificationText_.c_str(), -1, &measure,
            DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
        bool wrap = (measure.right - measure.left) > maxTextW || hudNotificationText_.find(L'\n') != std::wstring::npos;
        if (wrap) {
            measure = {0, 0, maxTextW, 0};
            DrawTextW(dc, hudNotificationText_.c_str(), -1, &measure,
                DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
        }
        int measuredW = static_cast<int>(measure.right - measure.left);
        int measuredH = static_cast<int>(measure.bottom - measure.top);
        int texW = std::clamp(measuredW + paddingX * 2, 220, maxTextW + paddingX * 2);
        int texH = std::clamp(measuredH + paddingY * 2, 58, 160);
        hudNotificationTextureWidth_ = texW;
        hudNotificationTextureHeight_ = texH;

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
        DrawTextW(dc, hudNotificationText_.c_str(), -1, &textRect, textFlags);
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
        if (FAILED(device_->CreateTexture2D(&td, &init, &hudNotificationTexture_))) return false;
        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MipLevels = 1;
        return SUCCEEDED(device_->CreateShaderResourceView(hudNotificationTexture_.Get(), &sd, &hudNotificationSrv_));
    }

    void DrawHudNotificationText(float x, float y, float w, float h, float alpha) {
        if (alpha <= 0.002f || !UpdateHudNotificationTexture() || !hudNotificationSrv_ ||
            !overlayBuffer_ || !texturedOverlayVertexShader_ || !texturedOverlayPixelShader_ ||
            !texturedOverlayInputLayout_ || !postSampler_) {
            return;
        }
        auto ndcX = [&](float px) { return px / static_cast<float>(width_) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(height_) * 2.0f; };
        TexturedOverlayVertex verts[6] = {
            {{ndcX(x), ndcY(y + h)}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x), ndcY(y)}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x + w), ndcY(y)}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x), ndcY(y + h)}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x + w), ndcY(y)}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, alpha}},
            {{ndcX(x + w), ndcY(y + h)}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, alpha}}
        };

        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context_->Map(overlayBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;
        std::memcpy(mapped.pData, verts, sizeof(verts));
        context_->Unmap(overlayBuffer_.Get(), 0);

        UINT stride = sizeof(TexturedOverlayVertex);
        UINT offset = 0;
        float blendFactor[4] = {};
        context_->IASetInputLayout(texturedOverlayInputLayout_.Get());
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->IASetVertexBuffers(0, 1, overlayBuffer_.GetAddressOf(), &stride, &offset);
        context_->VSSetShader(texturedOverlayVertexShader_.Get(), nullptr, 0);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        context_->PSSetShader(texturedOverlayPixelShader_.Get(), nullptr, 0);
        context_->OMSetDepthStencilState(depthDisabledState_.Get(), 0);
        context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
        ID3D11ShaderResourceView* srv = hudNotificationSrv_.Get();
        ID3D11SamplerState* sampler = postSampler_.Get();
        context_->PSSetShaderResources(0, 1, &srv);
        context_->PSSetSamplers(0, 1, &sampler);
        context_->Draw(6, 0);
        ID3D11ShaderResourceView* nullSrv = nullptr;
        context_->PSSetShaderResources(0, 1, &nullSrv);
    }

    void DrawPostProcess() {
        if (!sceneColorSrv_ || !postVertexShader_ || !postPixelShader_ || !postSampler_ || !rtv_) return;
        float blendFactor[4] = {};
        ID3D11RenderTargetView* target = rtv_.Get();
        context_->OMSetRenderTargets(1, &target, nullptr);
        context_->OMSetDepthStencilState(depthDisabledState_.Get(), 0);
        context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
        context_->RSSetState(rasterState_.Get());
        context_->IASetInputLayout(nullptr);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(postVertexShader_.Get(), nullptr, 0);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        context_->PSSetShader(postPixelShader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        ID3D11ShaderResourceView* srv = sceneColorSrv_.Get();
        ID3D11SamplerState* sampler = postSampler_.Get();
        context_->PSSetShaderResources(0, 1, &srv);
        context_->PSSetSamplers(0, 1, &sampler);
        context_->Draw(3, 0);
        ID3D11ShaderResourceView* nullSrv = nullptr;
        context_->PSSetShaderResources(0, 1, &nullSrv);
    }

    void DrawDreadMeterOverlay() {
        if (!settings_.dreadEnabled || !settings_.dreadDebugMeter || !overlayBuffer_ ||
            !overlayVertexShader_ || !overlayPixelShader_ || width_ <= 0 || height_ <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(48);
        auto ndcX = [&](float px) { return px / static_cast<float>(width_) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(height_) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };

        float x = 18.0f;
        float y = 18.0f;
        float w = 170.0f;
        float h = 13.0f;
        float fill = std::round((w - 4.0f) * Clamp01(dreadMeterLevel_));
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.07f, 0.025f, 0.025f, 0.74f});
        pushRect(x, y, w, h, {0.015f, 0.008f, 0.008f, 0.86f});
        if (fill > 0.5f) {
            pushRect(x + 2.0f, y + 2.0f, fill, h - 4.0f, {0.72f, 0.055f, 0.042f, 0.93f});
            pushRect(x + 2.0f, y + 2.0f, fill, 2.0f, {1.0f, 0.20f, 0.13f, 0.38f});
        }
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, 1.0f, {0.40f, 0.08f, 0.07f, 0.86f});
        pushRect(x - 2.0f, y + h + 1.0f, w + 4.0f, 1.0f, {0.23f, 0.035f, 0.035f, 0.86f});
        pushRect(x - 2.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});
        pushRect(x + w + 1.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});

        DrawOverlayVertices(verts);
    }

    void DrawGameHudOverlay() {
        if (!IsPlayableSimulationMode(runtimeMode_) || !overlayBuffer_ ||
            !overlayVertexShader_ || !overlayPixelShader_ || width_ <= 0 || height_ <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(96);
        auto ndcX = [&](float px) { return px / static_cast<float>(width_) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(height_) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };
        auto pushBar = [&](float x, float y, float w, float h, float fill, XMFLOAT4 fillColor) {
            fill = Clamp01(fill);
            pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.015f, 0.014f, 0.012f, 0.78f});
            pushRect(x, y, w, h, {0.03f, 0.028f, 0.024f, 0.84f});
            float fillW = std::round(w * fill);
            if (fillW > 0.5f) {
                pushRect(x, y, fillW, h, fillColor);
                pushRect(x, y, fillW, std::max(1.0f, h * 0.22f), {1.0f, 1.0f, 1.0f, 0.16f});
            }
        };

        if (runtimeMode_ == RendererRuntimeMode::PlayableGame && playableRun_.scoreScreenActive) {
            pushRect(0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_), {0.0f, 0.0f, 0.0f, 0.54f});
        }

        float x = 24.0f;
        float y = static_cast<float>(height_) - 58.0f;
        float w = std::clamp(static_cast<float>(width_) * 0.22f, 180.0f, 290.0f);
        pushBar(x, y, w, 14.0f, playerHealth_ / 100.0f, {0.72f, 0.05f, 0.045f, 0.92f});
        float staminaFill = playerStamina_ / 100.0f;
        float fatigue = 1.0f - SmoothStep(0.06f, 0.34f, staminaFill);
        XMFLOAT4 staminaColor{
            Lerp(0.88f, 0.92f, fatigue),
            Lerp(0.72f, 0.36f, fatigue),
            Lerp(0.23f, 0.08f, fatigue),
            0.88f
        };
        pushBar(x, y + 24.0f, w, 10.0f, staminaFill, staminaColor);
        float notificationAge = time_ - hudNotificationStartTime_;
        float notificationAlpha = 0.0f;
        float notificationX = 0.0f;
        float notificationY = 0.0f;
        float notificationW = 0.0f;
        float notificationH = 0.0f;
        if (!hudNotificationText_.empty() && notificationAge >= 0.0f && notificationAge < hudNotificationDuration_) {
            float inT = SmoothStep(0.0f, 0.22f, notificationAge);
            float outT = 1.0f - SmoothStep(std::max(0.0f, hudNotificationDuration_ - 0.65f), hudNotificationDuration_, notificationAge);
            notificationAlpha = Clamp01(inT * outT);
            float uiScale = std::clamp(static_cast<float>(height_) / 900.0f, 0.86f, 1.16f);
            notificationW = std::min(static_cast<float>(hudNotificationTextureWidth_) * uiScale, static_cast<float>(width_) - 84.0f);
            notificationH = static_cast<float>(hudNotificationTextureHeight_) * uiScale;
            notificationX = (static_cast<float>(width_) - notificationW) * 0.5f;
            notificationY = std::clamp(static_cast<float>(height_) * 0.145f, 64.0f, 150.0f);
            pushRect(notificationX - 14.0f, notificationY - 7.0f, notificationW + 28.0f, notificationH + 14.0f,
                {0.010f, 0.012f, 0.011f, 0.58f * notificationAlpha});
            pushRect(notificationX - 14.0f, notificationY - 7.0f, notificationW + 28.0f, 1.0f,
                {0.64f, 0.58f, 0.38f, 0.34f * notificationAlpha});
            pushRect(notificationX - 14.0f, notificationY + notificationH + 6.0f, notificationW + 28.0f, 1.0f,
                {0.64f, 0.58f, 0.38f, 0.22f * notificationAlpha});
        }
        DrawOverlayVertices(verts);
        if (notificationAlpha > 0.002f) {
            DrawHudNotificationText(notificationX, notificationY, notificationW, notificationH, notificationAlpha);
        }
    }

    void DrawMapOverlay() {
        if ((!settings_.mapOverlay && !settings_.debugAiMapOverlay) || !overlayBuffer_ || !overlayVertexShader_ || !overlayPixelShader_ ||
            width_ <= 0 || height_ <= 0 || maze_.w <= 0 || maze_.h <= 0 || maze_.open.empty()) {
            return;
        }

        bool cacheValid = !mapOverlayCachedVerts_.empty() &&
            mapOverlayCacheWidth_ == width_ &&
            mapOverlayCacheHeight_ == height_ &&
            mapOverlayCacheMazeW_ == maze_.w &&
            mapOverlayCacheMazeH_ == maze_.h &&
            mapOverlayCacheMode_ == runtimeMode_;
        if (cacheValid && time_ < mapOverlayNextUpdateTime_) {
            DrawOverlayVertices(mapOverlayCachedVerts_);
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(std::min(static_cast<size_t>(kOverlayVertexCapacity),
            static_cast<size_t>(maze_.w * maze_.h * 12 + monsterPath_.size() * 6 + 192)));
        auto ndcX = [&](float px) { return px / static_cast<float>(width_) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(height_) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            if (verts.size() + 6 > static_cast<size_t>(kOverlayVertexCapacity)) return;
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };

        float maxW = std::clamp(static_cast<float>(width_) * 0.22f, 125.0f, 245.0f);
        float maxH = std::clamp(static_cast<float>(height_) * 0.22f, 110.0f, 210.0f);
        float cell = std::max(1.15f, std::min(maxW / static_cast<float>(maze_.w), maxH / static_cast<float>(maze_.h)));
        float mapW = cell * static_cast<float>(maze_.w);
        float mapH = cell * static_cast<float>(maze_.h);
        float pad = 7.0f;
        float x0 = static_cast<float>(width_) - mapW - pad - 18.0f;
        float y0 = static_cast<float>(height_) - mapH - pad - 18.0f;
        Tile cameraTile = CameraTile();
        bool playerExplorationMap = runtimeMode_ == RendererRuntimeMode::PlayableGame && !settings_.debugAiMapOverlay;

        pushRect(x0 - pad, y0 - pad, mapW + pad * 2.0f, mapH + pad * 2.0f, {0.0f, 0.0f, 0.0f, 0.32f});
        pushRect(x0 - 1.0f, y0 - 1.0f, mapW + 2.0f, mapH + 2.0f, {0.53f, 0.46f, 0.31f, 0.24f});
        pushRect(x0, y0, mapW, mapH, {0.025f, 0.022f, 0.017f, 0.44f});
        auto mapTileX = [&](int tileX) {
            return x0 + static_cast<float>(maze_.w - 1 - tileX) * cell;
        };
        auto pushTile = [&](Tile t, XMFLOAT4 color, float insetScale = 0.14f) {
            if (!maze_.InBounds(t.x, t.y)) return;
            float inset = std::max(0.12f, cell * insetScale);
            float px = mapTileX(t.x);
            float py = y0 + static_cast<float>(t.y) * cell;
            pushRect(px + inset, py + inset, std::max(0.45f, cell - inset * 2.0f), std::max(0.45f, cell - inset * 2.0f), color);
        };
        auto markTile = [&](Tile t, XMFLOAT4 color, float scale) {
            if (!maze_.InBounds(t.x, t.y)) return;
            float size = std::max(3.0f, cell * scale);
            float px = mapTileX(t.x) + cell * 0.5f - size * 0.5f;
            float py = y0 + (static_cast<float>(t.y) + 0.5f) * cell - size * 0.5f;
            pushRect(px, py, size, size, color);
        };
        auto featureDiscovered = [&](Tile t) {
            if (!playerExplorationMap) return true;
            const Tile neighbors[] = {{t.x + 1, t.y}, {t.x - 1, t.y}, {t.x, t.y + 1}, {t.x, t.y - 1}};
            for (Tile n : neighbors) {
                if (!maze_.IsOpen(n.x, n.y)) continue;
                if (VisitCount(n) > 0 || n == cameraTile) return true;
            }
            return false;
        };

        for (int y = 0; y < maze_.h; ++y) {
            for (int x = 0; x < maze_.w; ++x) {
                MazeWallFeature feature = maze_.WallFeature(x, y);
                if (feature == MazeWallFeature::None) continue;
                Tile t{x, y};
                if (!featureDiscovered(t)) continue;
                XMFLOAT4 color = feature == MazeWallFeature::Window
                    ? XMFLOAT4{0.035f, 0.037f, 0.038f, 0.72f}
                    : XMFLOAT4{0.62f, 0.62f, 0.58f, 0.70f};
                float inset = std::max(0.16f, cell * 0.09f);
                float px = mapTileX(x);
                float py = y0 + static_cast<float>(y) * cell;
                pushRect(px + inset, py + inset, std::max(0.55f, cell - inset * 2.0f), std::max(0.55f, cell - inset * 2.0f), color);
            }
        }

        for (int y = 0; y < maze_.h; ++y) {
            for (int x = 0; x < maze_.w; ++x) {
                if (!maze_.IsOpen(x, y)) continue;
                Tile t{x, y};
                if (playerExplorationMap && VisitCount(t) == 0 && !(t == cameraTile)) continue;
                XMFLOAT4 color{0.74f, 0.66f, 0.47f, playerExplorationMap ? 0.30f : 0.36f};
                if (t == maze_.exit) color = {0.20f, 0.88f, 0.38f, 0.78f};
                float px = mapTileX(x);
                float py = y0 + static_cast<float>(y) * cell;
                float inset = std::max(0.18f, cell * 0.10f);
                pushRect(px + inset, py + inset, std::max(0.6f, cell - inset * 2.0f), std::max(0.6f, cell - inset * 2.0f), color);
            }
        }

        if (IsPlayableSimulationMode(runtimeMode_) || settings_.debugAiMapOverlay) {
            for (const PlayerAudibleSoundPulse& pulse : playerAudibleSoundPulses_) {
                if (pulse.radius <= 0.05f || pulse.life <= 0.0f || pulse.age >= pulse.life) continue;
                float radiusSq = pulse.radius * pulse.radius;
                float fade = 1.0f - Clamp01(pulse.age / pulse.life);
                XMFLOAT4 hearingColor = pulse.heardByMonster
                    ? XMFLOAT4{1.0f, 0.02f, 0.02f, 0.18f + 0.30f * fade}
                    : XMFLOAT4{1.0f, 0.03f, 0.02f, 0.10f + 0.22f * fade};
                for (int y = 0; y < maze_.h; ++y) {
                    for (int x = 0; x < maze_.w; ++x) {
                        if (!maze_.IsOpen(x, y)) continue;
                        Tile t{x, y};
                        if (playerExplorationMap && VisitCount(t) == 0 && !(t == cameraTile)) continue;
                        XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
                        float dx = center.x - pulse.pos.x;
                        float dz = center.z - pulse.pos.z;
                        if (dx * dx + dz * dz <= radiusSq) {
                            pushTile(t, hearingColor, 0.02f);
                        }
                    }
                }
            }
        }

        bool drawAiDebug = settings_.debugAiMapOverlay ||
            (runtimeMode_ == RendererRuntimeMode::ScreensaverAutopilot && settings_.mapOverlay);
        if (drawAiDebug) {
            bool alertPath = monsterHasSound_ || monsterHasLastKnown_ || monsterChasingVisible_;
            XMFLOAT4 pathColor = alertPath ? XMFLOAT4{1.0f, 0.12f, 0.04f, 0.72f} : XMFLOAT4{1.0f, 0.56f, 0.10f, 0.52f};
            size_t pathRemaining = monsterPath_.size() > monsterPathIndex_ ? monsterPath_.size() - monsterPathIndex_ : 0;
            size_t pathStep = std::max<size_t>(1, (pathRemaining + 95) / 96);
            for (size_t i = monsterPathIndex_; i < monsterPath_.size(); i += pathStep) {
                float t = monsterPath_.size() > monsterPathIndex_
                    ? static_cast<float>(i - monsterPathIndex_) / static_cast<float>(std::max<size_t>(1, monsterPath_.size() - monsterPathIndex_))
                    : 0.0f;
                XMFLOAT4 color = pathColor;
                color.w *= Lerp(1.0f, 0.46f, Clamp01(t));
                pushTile(monsterPath_[i], color, 0.26f);
            }
            if (monsterHeardPlayerNow_) markTile(CameraTile(), {1.0f, 0.0f, 0.0f, 1.0f}, 2.35f);
            if (monsterHasSound_) markTile(monsterSoundTile_, {1.0f, 0.02f, 0.02f, 0.96f}, 2.05f);
            if (monsterHasLastKnown_) markTile(monsterLastKnownTile_, {1.0f, 0.88f, 0.16f, 0.90f}, 1.85f);
            if (ValidMonsterTile(monsterGoal_)) markTile(monsterGoal_, {1.0f, 0.38f, 0.02f, 0.78f}, 1.62f);
            if (ValidMonsterTile(monsterRoamTile_) && !monsterHasSound_ && !monsterHasLastKnown_) {
                markTile(monsterRoamTile_, {0.82f, 0.68f, 0.42f, 0.56f}, 1.42f);
            }
            for (Tile occupied : MonsterBodyOccupiedTiles()) {
                markTile(occupied, {0.88f, 0.02f, 0.04f, 0.58f}, 1.34f);
            }
            markTile(MonsterTile(), {1.0f, 0.02f, 0.02f, 0.78f}, 1.55f);
        }

        markTile(cameraTile, {0.20f, 0.72f, 1.0f, 0.82f}, 1.70f);

        mapOverlayCachedVerts_ = verts;
        mapOverlayCacheWidth_ = width_;
        mapOverlayCacheHeight_ = height_;
        mapOverlayCacheMazeW_ = maze_.w;
        mapOverlayCacheMazeH_ = maze_.h;
        mapOverlayCacheMode_ = runtimeMode_;
        mapOverlayNextUpdateTime_ = time_ + (settings_.debugAiMapOverlay ? 0.085f : 0.16f);
        DrawOverlayVertices(verts);
    }
