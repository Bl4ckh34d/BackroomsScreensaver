    bool CustomGameControlPlacement(CustomGameMenuControl control, MenuPlaquePlacement& out) const {
        RECT r{};
        if (!CustomGameControlPixelRect(control, r)) return false;
        MenuPlaquePlacement panel = MenuCustomPanelPlacement();
        constexpr float logicalSize = 512.0f;
        float centerX = ((static_cast<float>(r.left + r.right) * 0.5f) / logicalSize - 0.5f) * panel.halfW * 2.0f;
        float centerY = (0.5f - (static_cast<float>(r.top + r.bottom) * 0.5f) / logicalSize) * panel.halfH * 2.0f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        out = panel;
        out.center = Add3(panel.center, Add3(Add3(Scale3(panel.right, centerX), Scale3(up, centerY)), Scale3(panel.inward, 0.082f)));
        out.halfW = (static_cast<float>(r.right - r.left) / logicalSize) * panel.halfW;
        out.halfH = (static_cast<float>(r.bottom - r.top) / logicalSize) * panel.halfH;
        return out.halfW > 0.0f && out.halfH > 0.0f;
    }

    bool ProjectMenuQuadToScreen(XMFLOAT3 center, XMFLOAT3 right, XMFLOAT3 up, float halfW, float halfH, RECT& out) const {
        if (hostRuntime_.width <= 0 || hostRuntime_.height <= 0) return false;
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});

        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 f = Forward();
        XMVECTOR eye = XMLoadFloat3(&world.playerPosition);
        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        XMVECTOR viewDir = XMVector3Normalize(XMVectorSet(f.x, world.playerPitch, f.z, 0.0f));
        XMMATRIX view = XMMatrixLookAtLH(eye, eye + viewDir, worldUp);
        float aspect = static_cast<float>(std::max<LONG>(1, hostRuntime_.width)) / static_cast<float>(std::max<LONG>(1, hostRuntime_.height));
        float fovDegrees = sessionRuntime_.mode == RendererRuntimeMode::MainMenu ? 84.0f : 70.0f;
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovDegrees * kPi / 180.0f, aspect, 0.045f, 42.0f);
        XMMATRIX viewProj = view * proj;

        auto p = [&](float x, float y) {
            return Add3(center, Add3(Scale3(right, x * halfW), Scale3(up, y * halfH)));
        };
        std::array<XMFLOAT3, 4> corners = {{
            p(-1.0f, -1.0f),
            p( 1.0f, -1.0f),
            p( 1.0f,  1.0f),
            p(-1.0f,  1.0f)
        }};

        float minX = static_cast<float>(hostRuntime_.width);
        float minY = static_cast<float>(hostRuntime_.height);
        float maxX = 0.0f;
        float maxY = 0.0f;
        int visible = 0;
        for (const XMFLOAT3& corner : corners) {
            XMVECTOR clip = XMVector3TransformCoord(XMLoadFloat3(&corner), viewProj);
            XMFLOAT3 ndc{};
            XMStoreFloat3(&ndc, clip);
            if (ndc.z < 0.0f || ndc.z > 1.0f) continue;
            float sx = (ndc.x * 0.5f + 0.5f) * static_cast<float>(hostRuntime_.width);
            float sy = (0.5f - ndc.y * 0.5f) * static_cast<float>(hostRuntime_.height);
            minX = std::min(minX, sx);
            minY = std::min(minY, sy);
            maxX = std::max(maxX, sx);
            maxY = std::max(maxY, sy);
            ++visible;
        }
        if (visible < 3) return false;

        constexpr LONG pad = 4;
        out.left = std::clamp(static_cast<LONG>(std::floor(minX)) - pad, 0L, static_cast<LONG>(hostRuntime_.width));
        out.top = std::clamp(static_cast<LONG>(std::floor(minY)) - pad, 0L, static_cast<LONG>(hostRuntime_.height));
        out.right = std::clamp(static_cast<LONG>(std::ceil(maxX)) + pad, 0L, static_cast<LONG>(hostRuntime_.width));
        out.bottom = std::clamp(static_cast<LONG>(std::ceil(maxY)) + pad, 0L, static_cast<LONG>(hostRuntime_.height));
        return out.right > out.left && out.bottom > out.top;
    }
