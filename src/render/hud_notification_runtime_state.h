#pragma once

struct HudNotificationRuntimeState {
    std::wstring text;
    float startTime = -1000.0f;
    float duration = 0.0f;
    bool textureDirty = false;
    int textureWidth = 1024;
    int textureHeight = 160;
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> srv;
};
