#pragma once

struct SparkEmitter {
    XMFLOAT3 pos{};
    float cooldown = 0.0f;
    bool triggered = false;
};

struct SparkParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 1.0f;
    float size = 0.025f;
};

struct SparkFlash {
    XMFLOAT3 pos{};
    float age = 0.0f;
    float life = 0.18f;
    float intensity = 1.0f;
};

struct SparkChain {
    XMFLOAT3 pos{};
    float timer = 0.0f;
    float intensity = 1.0f;
    int remaining = 0;
};

struct RuntimeLampState {
    Tile tile{};
    XMFLOAT3 pos{};
    float damage = 0.0f;
    float sparkTimer = 0.0f;
    bool broken = false;
    int humVariant = 0;
    bool flickerWasDim = false;
    float flickerClickCooldown = 0.0f;
};

struct SteamEmitter {
    XMFLOAT3 pos{};
    XMFLOAT3 dir{0.0f, 0.0f, 1.0f};
    float cooldown = 0.0f;
    bool panelDropped = false;
    bool triggered = false;
};

struct WetDripEmitter {
    XMFLOAT3 pos{};
    float interval = 1.0f;
    float timer = 0.0f;
    float volume = 0.30f;
    float age = 0.0f;
    float audibleDelay = 0.0f;
};

struct WetFloorFootprint {
    XMFLOAT2 center{};
    XMFLOAT2 right{1.0f, 0.0f};
    XMFLOAT2 forward{0.0f, 1.0f};
    float halfW = 0.0f;
    float halfD = 0.0f;
    float wetDelaySeconds = 0.0f;
};

struct SteamParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 1.0f;
    float size = 0.25f;
};

struct VentDrop {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float yaw = 0.0f;
    float roll = 0.0f;
    float angular = 0.0f;
    float age = 0.0f;
    float life = 3.0f;
    float halfW = 0.46f;
    float halfH = 0.27f;
    bool landed = false;
};

struct AirParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 32.0f;
    float size = 0.018f;
    float seed = 0.0f;
    float angle = 0.0f;
    float spin = 0.0f;
    float aspect = 1.0f;
    float nearLayer = 0.0f;
};

struct BloodScarePoint {
    XMFLOAT3 pos{};
    XMFLOAT3 source{};
    XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
    float radius = 2.2f;
    float activationTime = -1000000.0f;
    float focusDelaySeconds = 1.20f;
    float dreadScale = 1.0f;
    bool triggered = false;
    bool focusTaken = false;
    bool requireFacing = false;
    bool revealBlood = true;
    bool waterLiquid = false;
};

struct BloodRevealRegion {
    XMFLOAT3 center{};
    float radius = 0.0f;
    float activationTime = -1000000.0f;
    bool waterLiquid = false;
};
