#pragma once

struct Maze;

struct PlayerWindowVault {
    XMFLOAT3 start{};
    XMFLOAT3 entry{};
    XMFLOAT3 exit{};
    XMFLOAT3 landing{};
    float crossingEye = 0.0f;
    float leanSide = 1.0f;
    float progress = 0.0f;
    float crawlSpeed = 0.0f;
    bool returning = false;
    bool active = false;
};

struct WindowVaultCameraPose {
    float pitch = 0.0f;
    float roll = 0.0f;
};

bool PlanWindowVault(const Maze& maze, const XMFLOAT3& position, float yaw,
                     float wallHeight, float landingEye, PlayerWindowVault& vault);
void AdvanceWindowVault(PlayerWindowVault& vault, float dt, float forwardInput, float crawlSpeed);
XMFLOAT3 SampleWindowVault(const PlayerWindowVault& vault, float progress);
WindowVaultCameraPose SampleWindowVaultCamera(const PlayerWindowVault& vault, float progress);
