    const wchar_t* GpuProfileMarkerName(GpuProfileMarker marker) const {
        switch (marker) {
        case GpuProfileMarker::FrameStart: return L"FrameStart";
        case GpuProfileMarker::ClearTargets: return L"ClearTargets";
        case GpuProfileMarker::DynamicGeometry: return L"DynamicGeometry";
        case GpuProfileMarker::FlashlightShadow: return L"FlashlightShadow";
        case GpuProfileMarker::FixtureShadow: return L"FixtureShadow";
        case GpuProfileMarker::MonsterEyeShadow: return L"MonsterEyeShadow";
        case GpuProfileMarker::Uploads: return L"Uploads";
        case GpuProfileMarker::MainOpaque: return L"MainOpaque";
        case GpuProfileMarker::FloorCeiling: return L"FloorCeiling";
        case GpuProfileMarker::DynamicOpaque: return L"DynamicOpaque";
        case GpuProfileMarker::StaticWater: return L"StaticWater";
        case GpuProfileMarker::StaticTransparent: return L"StaticTransparent";
        case GpuProfileMarker::DynamicTransparent: return L"DynamicTransparent";
        case GpuProfileMarker::PostProcess: return L"PostProcess";
        case GpuProfileMarker::Overlays: return L"Overlays";
        case GpuProfileMarker::FrameEnd: return L"FrameEnd";
        default: return L"Unknown";
        }
    }

    void CreateGpuProfileQueries() {
        gpuProfileAvailable_ = false;
        gpuProfileFrameOpen_ = false;
        gpuProfileWriteIndex_ = 0;
        gpuProfileFrameCounter_ = 0;
        for (GpuProfileFrame& frame : gpuProfileFrames_) {
            frame = {};
        }
        if (!StartupProfileEnabled() || !device_) return;

        D3D11_QUERY_DESC disjointDesc{};
        disjointDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        D3D11_QUERY_DESC timestampDesc{};
        timestampDesc.Query = D3D11_QUERY_TIMESTAMP;

        for (GpuProfileFrame& frame : gpuProfileFrames_) {
            if (FAILED(device_->CreateQuery(&disjointDesc, &frame.disjoint))) {
                StartupProfileLine(L"GPU profile queries unavailable: timestamp disjoint query creation failed.");
                for (GpuProfileFrame& resetFrame : gpuProfileFrames_) resetFrame = {};
                return;
            }
            for (ComPtr<ID3D11Query>& timestamp : frame.timestamps) {
                if (FAILED(device_->CreateQuery(&timestampDesc, &timestamp))) {
                    StartupProfileLine(L"GPU profile queries unavailable: timestamp query creation failed.");
                    for (GpuProfileFrame& resetFrame : gpuProfileFrames_) resetFrame = {};
                    return;
                }
            }
        }
        gpuProfileAvailable_ = true;
        StartupProfileLine(L"GPU profile queries ready.");
    }

    void ResolveGpuProfileFrame(GpuProfileFrame& frame) {
        if (!gpuProfileAvailable_ || !context_ || !frame.issued || frame.open) return;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
        HRESULT hr = context_->GetData(frame.disjoint.Get(), &disjoint, sizeof(disjoint), D3D11_ASYNC_GETDATA_DONOTFLUSH);
        if (hr == S_FALSE) return;
        if (FAILED(hr)) {
            frame.issued = false;
            StartupProfileLine(L"GPU profile frame dropped: disjoint query read failed.");
            return;
        }
        if (disjoint.Disjoint || disjoint.Frequency == 0) {
            frame.issued = false;
            StartupProfileLine(L"GPU profile frame dropped: timestamp frequency was disjoint.");
            return;
        }

        std::array<UINT64, kGpuProfileMarkerCount> timestamps{};
        for (size_t i = 0; i < kGpuProfileMarkerCount; ++i) {
            hr = context_->GetData(frame.timestamps[i].Get(), &timestamps[i], sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if (hr == S_FALSE) return;
            if (FAILED(hr)) {
                frame.issued = false;
                StartupProfileLine(L"GPU profile frame dropped: timestamp read failed.");
                return;
            }
        }

        auto elapsedMs = [&](size_t from, size_t to) {
            if (timestamps[to] < timestamps[from]) return 0.0;
            return static_cast<double>(timestamps[to] - timestamps[from]) * 1000.0 /
                static_cast<double>(disjoint.Frequency);
        };

        std::wostringstream line;
        line << std::fixed << std::setprecision(3);
        line << L"GPU frame " << frame.frameId << L": total="
             << elapsedMs(static_cast<size_t>(GpuProfileMarker::FrameStart), static_cast<size_t>(GpuProfileMarker::FrameEnd))
             << L" ms";
        for (size_t i = 1; i < kGpuProfileMarkerCount; ++i) {
            line << L", "
                 << GpuProfileMarkerName(static_cast<GpuProfileMarker>(i))
                 << L"=" << elapsedMs(i - 1, i) << L" ms";
        }
        StartupProfileLine(line.str());
        frame.issued = false;
    }

    void BeginGpuProfileFrame() {
        if (!gpuProfileAvailable_ || !context_ || gpuProfileFrameOpen_) return;
        for (GpuProfileFrame& frame : gpuProfileFrames_) {
            ResolveGpuProfileFrame(frame);
        }

        GpuProfileFrame& frame = gpuProfileFrames_[gpuProfileWriteIndex_];
        if (frame.issued || !frame.disjoint) return;

        frame.open = true;
        frame.frameId = ++gpuProfileFrameCounter_;
        context_->Begin(frame.disjoint.Get());
        gpuProfileFrameOpen_ = true;
        MarkGpuProfile(GpuProfileMarker::FrameStart);
    }

    void MarkGpuProfile(GpuProfileMarker marker) {
        if (!gpuProfileAvailable_ || !context_ || !gpuProfileFrameOpen_) return;
        GpuProfileFrame& frame = gpuProfileFrames_[gpuProfileWriteIndex_];
        size_t index = static_cast<size_t>(marker);
        if (index >= kGpuProfileMarkerCount || !frame.timestamps[index]) return;
        context_->End(frame.timestamps[index].Get());
    }

    void EndGpuProfileFrame() {
        if (!gpuProfileAvailable_ || !context_ || !gpuProfileFrameOpen_) return;
        GpuProfileFrame& frame = gpuProfileFrames_[gpuProfileWriteIndex_];
        MarkGpuProfile(GpuProfileMarker::FrameEnd);
        context_->End(frame.disjoint.Get());
        frame.open = false;
        frame.issued = true;
        gpuProfileFrameOpen_ = false;
        gpuProfileWriteIndex_ = (gpuProfileWriteIndex_ + 1) % kGpuProfileFrameCount;
    }

    bool CreateStates() {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_ANISOTROPIC;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxAnisotropy = 8;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&sd, &sampler_))) return false;

        D3D11_SAMPLER_DESC shadowSd{};
        shadowSd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        shadowSd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.BorderColor[0] = 1.0f;
        shadowSd.BorderColor[1] = 1.0f;
        shadowSd.BorderColor[2] = 1.0f;
        shadowSd.BorderColor[3] = 1.0f;
        shadowSd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        shadowSd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&shadowSd, &shadowSampler_))) return false;

        D3D11_SAMPLER_DESC postSd{};
        postSd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        postSd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&postSd, &postSampler_))) return false;

        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.DepthClipEnable = TRUE;
        if (FAILED(device_->CreateRasterizerState(&rd, &rasterState_))) return false;

        D3D11_RASTERIZER_DESC shadowRd = rd;
        shadowRd.DepthBias = 32;
        shadowRd.SlopeScaledDepthBias = 1.6f;
        shadowRd.DepthBiasClamp = 0.0025f;
        if (FAILED(device_->CreateRasterizerState(&shadowRd, &shadowRasterState_))) return false;

        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = TRUE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        if (FAILED(device_->CreateDepthStencilState(&dsd, &depthState_))) return false;
        D3D11_DEPTH_STENCIL_DESC lessDsd = dsd;
        lessDsd.DepthFunc = D3D11_COMPARISON_LESS;
        if (FAILED(device_->CreateDepthStencilState(&lessDsd, &depthLessState_))) return false;
        D3D11_DEPTH_STENCIL_DESC readOnlyDsd = dsd;
        readOnlyDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (FAILED(device_->CreateDepthStencilState(&readOnlyDsd, &depthReadOnlyState_))) return false;
        D3D11_DEPTH_STENCIL_DESC liquidDsd = lessDsd;
        liquidDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        if (FAILED(device_->CreateDepthStencilState(&liquidDsd, &liquidDepthStencilState_))) return false;
        D3D11_DEPTH_STENCIL_DESC overlayDsd{};
        overlayDsd.DepthEnable = FALSE;
        overlayDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        overlayDsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
        if (FAILED(device_->CreateDepthStencilState(&overlayDsd, &depthDisabledState_))) return false;

        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        if (FAILED(device_->CreateBlendState(&bd, &alphaBlend_))) return false;
        CreateGpuProfileQueries();
        return true;
    }

    bool CreateShadowResources() {
        shadowMapSize_ = static_cast<UINT>(std::clamp(settings_.flashlightShadowMapSize, 512, 4096));
        fixtureShadowMapSize_ = static_cast<UINT>(std::clamp(settings_.flashlightShadowMapSize / 4, 512, 1024));
        monsterEyeShadowMapSize_ = static_cast<UINT>(std::clamp(settings_.flashlightShadowMapSize / 2, 512, 2048));

        D3D11_TEXTURE2D_DESC td{};
        td.Width = shadowMapSize_;
        td.Height = shadowMapSize_;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32_TYPELESS;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(device_->CreateTexture2D(&td, nullptr, &shadowDepth_))) return false;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        if (FAILED(device_->CreateDepthStencilView(shadowDepth_.Get(), &dsvDesc, &shadowDsv_))) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        if (FAILED(device_->CreateShaderResourceView(shadowDepth_.Get(), &srvDesc, &shadowSrv_))) return false;

        td.Width = fixtureShadowMapSize_;
        td.Height = fixtureShadowMapSize_;
        fixtureShadowDepth_.Reset();
        fixtureShadowDsv_.Reset();
        fixtureShadowSrv_.Reset();
        if (FAILED(device_->CreateTexture2D(&td, nullptr, &fixtureShadowDepth_)) ||
            FAILED(device_->CreateDepthStencilView(fixtureShadowDepth_.Get(), &dsvDesc, &fixtureShadowDsv_)) ||
            FAILED(device_->CreateShaderResourceView(fixtureShadowDepth_.Get(), &srvDesc, &fixtureShadowSrv_))) {
            fixtureShadowDepth_.Reset();
            fixtureShadowDsv_.Reset();
            fixtureShadowSrv_.Reset();
            StartupProfileLine(L"Fixture shadow resources unavailable; continuing without fixture-cast shadows.");
        }

        td.Width = monsterEyeShadowMapSize_;
        td.Height = monsterEyeShadowMapSize_;
        for (size_t i = 0; i < monsterEyeShadowDepth_.size(); ++i) {
            monsterEyeShadowDepth_[i].Reset();
            monsterEyeShadowDsv_[i].Reset();
            monsterEyeShadowSrv_[i].Reset();
            if (FAILED(device_->CreateTexture2D(&td, nullptr, &monsterEyeShadowDepth_[i]))) return false;
            if (FAILED(device_->CreateDepthStencilView(monsterEyeShadowDepth_[i].Get(), &dsvDesc, &monsterEyeShadowDsv_[i]))) return false;
            if (FAILED(device_->CreateShaderResourceView(monsterEyeShadowDepth_[i].Get(), &srvDesc, &monsterEyeShadowSrv_[i]))) return false;
        }
        return true;
    }

    bool CreateConstantBuffer() {
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(SceneConstants);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        return SUCCEEDED(device_->CreateBuffer(&bd, nullptr, &constantBuffer_));
    }
