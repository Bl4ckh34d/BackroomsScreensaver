#pragma once

struct HudNotificationRuntimeState {
    std::wstring text;
    float startTime = -1000.0f;
    float duration = 0.0f;
    float fadeInSeconds = 0.22f;
    bool centered = false;
    bool persistent = false;
    bool textureDirty = false;
    int textureWidth = 1024;
    int textureHeight = 160;
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> srv;
};
