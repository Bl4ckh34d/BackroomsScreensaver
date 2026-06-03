        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 2;
        scd.BufferDesc.Width = static_cast<UINT>(hostRuntime_.width);
        scd.BufferDesc.Height = static_cast<UINT>(hostRuntime_.height);
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hostRuntime_.hwnd;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
            &scd, &d3dRuntime_.swapChain, &d3dRuntime_.device, &d3dRuntime_.featureLevel, &d3dRuntime_.context);
        if (FAILED(hr) && settingsRuntime_.live.allowWarpFallback) {
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                &scd, &d3dRuntime_.swapChain, &d3dRuntime_.device, &d3dRuntime_.featureLevel, &d3dRuntime_.context);
        }
        if (FAILED(hr)) {
            startupRuntime_.lastInitializeError = L"CreateDeviceAndSwapChain failed: HRESULT 0x" + [&]() {
                std::wstringstream ss;
                ss << std::hex << static_cast<unsigned long>(hr);
                return ss.str();
            }();
            return false;
        }
        profile.Mark(L"CreateDeviceAndSwapChain");
        startupRuntime_.shaderTotal = d3dRuntime_.featureLevel >= D3D_FEATURE_LEVEL_11_0 ? 9 : 7;
        startupRuntime_.total = kStartupProgressPreShaderSteps + startupRuntime_.shaderTotal + kStartupProgressPostShaderSteps;
        startupRuntime_.fineTotal = startupRuntime_.total * kStartupProgressUnitsPerStep;
        ReportStartupStep(L"Direct3D device ready", L"Creating render targets.");
