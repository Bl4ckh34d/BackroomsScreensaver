#pragma once

struct RenderBufferRuntimeState {
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    ComPtr<ID3D11Buffer> instancedVertexBuffer;
    ComPtr<ID3D11Buffer> instancedIndexBuffer;
    ComPtr<ID3D11Buffer> instancedInstanceBuffer;
    ComPtr<ID3D11Buffer> monsterBuffer;
    ComPtr<ID3D11Buffer> dynamicBuffer;
    ComPtr<ID3D11Buffer> overlayBuffer;
    ComPtr<ID3D11Buffer> constantBuffer;
};

struct MaterialTextureRuntimeState {
    ComPtr<ID3D11ShaderResourceView> albedoSrv;
    ComPtr<ID3D11ShaderResourceView> normalSrv;
    ComPtr<ID3D11ShaderResourceView> materialPropsSrv;
    ComPtr<ID3D11ShaderResourceView> ceilingAlbedoSrv;
    ComPtr<ID3D11ShaderResourceView> ceilingNormalSrv;
    ComPtr<ID3D11ShaderResourceView> ceilingPropsSrv;
    ComPtr<ID3D11ShaderResourceView> doorAlbedoSrv;
    ComPtr<ID3D11ShaderResourceView> doorNormalSrv;
    ComPtr<ID3D11ShaderResourceView> doorPropsSrv;
    ComPtr<ID3D11ShaderResourceView> doorFrameAlbedoSrv;
    ComPtr<ID3D11ShaderResourceView> doorFrameNormalSrv;
    ComPtr<ID3D11ShaderResourceView> doorFramePropsSrv;
};

struct RuntimeTextureResourceState {
    ComPtr<ID3D11Texture2D> customMenuTexture;
    ComPtr<ID3D11ShaderResourceView> customMenuSrv;
    ComPtr<ID3D11ShaderResourceView> loosePagesSrv;
    ComPtr<ID3D11ShaderResourceView> flashlightPatternSrv;
    ComPtr<ID3D11ShaderResourceView> mazeSrv;
    ComPtr<ID3D11Texture2D> lampDamageTexture;
    ComPtr<ID3D11ShaderResourceView> lampDamageSrv;
};
