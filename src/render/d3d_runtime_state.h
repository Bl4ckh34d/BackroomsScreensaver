#pragma once

struct RendererDeviceRuntimeState {
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;
};

struct RenderTargetRuntimeState {
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11Texture2D> depth;
    ComPtr<ID3D11DepthStencilView> dsv;
    ComPtr<ID3D11Texture2D> sceneColor;
    ComPtr<ID3D11RenderTargetView> sceneColorRtv;
    ComPtr<ID3D11ShaderResourceView> sceneColorSrv;
};

struct ShadowResourceRuntimeState {
    ComPtr<ID3D11Texture2D> shadowDepth;
    ComPtr<ID3D11DepthStencilView> shadowDsv;
    ComPtr<ID3D11ShaderResourceView> shadowSrv;
    ComPtr<ID3D11Texture2D> fixtureShadowDepth;
    ComPtr<ID3D11DepthStencilView> fixtureShadowDsv;
    ComPtr<ID3D11ShaderResourceView> fixtureShadowSrv;
    UINT shadowMapSize = 2048;
    UINT fixtureShadowMapSize = 1024;
};
