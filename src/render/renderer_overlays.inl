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

        float x = 24.0f;
        float y = static_cast<float>(height_) - 58.0f;
        float w = std::clamp(static_cast<float>(width_) * 0.22f, 180.0f, 290.0f);
        pushBar(x, y, w, 14.0f, playerHealth_ / 100.0f, {0.72f, 0.05f, 0.045f, 0.92f});
        pushBar(x, y + 24.0f, w, 10.0f, playerStamina_ / 100.0f, {0.88f, 0.72f, 0.23f, 0.88f});
        DrawOverlayVertices(verts);
    }

    void DrawMapOverlay() {
        if ((!settings_.mapOverlay && !settings_.debugAiMapOverlay) || !overlayBuffer_ || !overlayVertexShader_ || !overlayPixelShader_ ||
            width_ <= 0 || height_ <= 0 || maze_.w <= 0 || maze_.h <= 0 || maze_.open.empty()) {
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
            for (size_t i = monsterPathIndex_; i < monsterPath_.size(); ++i) {
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

        DrawOverlayVertices(verts);
    }
