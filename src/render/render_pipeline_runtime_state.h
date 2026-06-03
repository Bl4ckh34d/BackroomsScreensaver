#pragma once

struct ShaderRuntimeState {
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11HullShader> hullShader;
    ComPtr<ID3D11DomainShader> domainShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11VertexShader> overlayVertexShader;
    ComPtr<ID3D11PixelShader> overlayPixelShader;
    ComPtr<ID3D11VertexShader> texturedOverlayVertexShader;
    ComPtr<ID3D11PixelShader> texturedOverlayPixelShader;
    ComPtr<ID3D11VertexShader> postVertexShader;
    ComPtr<ID3D11PixelShader> postPixelShader;
    ComPtr<ID3D11VertexShader> instancedVertexShader;
    ComPtr<ID3D11PixelShader> liquidPixelShader;
    ComPtr<ID3D11PixelShader> shadowPixelShader;
};

struct InputLayoutRuntimeState {
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11InputLayout> instancedInputLayout;
    ComPtr<ID3D11InputLayout> overlayInputLayout;
    ComPtr<ID3D11InputLayout> texturedOverlayInputLayout;
};

struct PipelineStateRuntimeState {
    ComPtr<ID3D11SamplerState> sampler;
    ComPtr<ID3D11SamplerState> shadowSampler;
    ComPtr<ID3D11SamplerState> postSampler;
    ComPtr<ID3D11RasterizerState> rasterState;
    ComPtr<ID3D11RasterizerState> shadowRasterState;
    ComPtr<ID3D11DepthStencilState> depthState;
    ComPtr<ID3D11DepthStencilState> depthLessState;
    ComPtr<ID3D11DepthStencilState> depthReadOnlyState;
    ComPtr<ID3D11DepthStencilState> depthDisabledState;
    ComPtr<ID3D11DepthStencilState> liquidDepthStencilState;
    ComPtr<ID3D11BlendState> alphaBlend;
};
