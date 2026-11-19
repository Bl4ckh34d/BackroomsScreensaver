#include "../platform/platform_headers.h"
#include "../core/math_utils.h"
#include "../maze/maze.h"
#include "player_window_vault.h"

namespace {
constexpr float kClimbEnd = 0.26f;
constexpr float kCrawlEnd = 0.75f;
constexpr float kAnimationSeconds = 1.02f;

float MotionPulse(float start, float peak, float end, float progress) {
    return SmoothStep(start, peak, progress) * (1.0f - SmoothStep(peak, end, progress));
}
}

bool PlanWindowVault(const Maze& maze, const XMFLOAT3& position, float yaw,
                     float wallHeight, float landingEye, PlayerWindowVault& vault) {
    const Tile start = maze.TileFromWorld(position.x, position.z);
    if (!maze.IsOpen(start.x, start.y)) return false;

    const float forwardX = std::sin(yaw);
    const float forwardZ = std::cos(yaw);
    if (std::max(std::abs(forwardX), std::abs(forwardZ)) < 0.76f) return false;
    const bool alongX = std::abs(forwardX) > std::abs(forwardZ);
    const Tile step = alongX
        ? Tile{forwardX > 0.0f ? 1 : -1, 0}
        : Tile{0, forwardZ > 0.0f ? 1 : -1};
    const Tile opening{start.x + step.x, start.y + step.y};
    const Tile beyond{opening.x + step.x, opening.y + step.y};
    if (maze.WallFeature(opening.x, opening.y) != MazeWallFeature::Window ||
        !maze.IsOpen(beyond.x, beyond.y)) return false;

    const XMFLOAT3 center = maze.WorldCenter(opening);
    const float depth = alongX ? maze.tileW : maze.tileD;
    const float radius = std::clamp(maze.TileMinimum() * 0.26f, 0.12f, 0.46f);
    const float clearance = depth * 0.5f + radius + 0.045f;
    const XMFLOAT3 entry{center.x - step.x * clearance, position.y, center.z - step.y * clearance};
    if (std::hypot(entry.x - position.x, entry.z - position.z) > 1.0f) return false;

    const float sill = MazeWindowSillHeight(wallHeight);
    const float crossingEye = std::min(wallHeight - 0.19f, std::max(landingEye + 0.20f, sill + 0.47f));
    if (crossingEye - sill < 0.29f) return false;
    const XMFLOAT3 exit{center.x + step.x * clearance, crossingEye, center.z + step.y * clearance};
    const float lateralOffset = (position.x - center.x) * step.y - (position.z - center.z) * step.x;
    vault = {
        position, entry, exit,
        {exit.x + step.x * 0.17f, landingEye, exit.z + step.y * 0.17f},
        crossingEye, lateralOffset < -0.045f ? -1.0f : 1.0f
    };
    vault.active = true;
    return true;
}

void AdvanceWindowVault(PlayerWindowVault& vault, float dt, float forwardInput, float crawlSpeed) {
    if (vault.returning) {
        vault.progress = std::max(0.0f, vault.progress - dt / kAnimationSeconds);
        vault.active = vault.progress > 0.0f;
    } else if (vault.progress < kClimbEnd) {
        vault.progress = std::min(kClimbEnd, vault.progress + dt / kAnimationSeconds);
    } else if (vault.progress < kCrawlEnd) {
        const float targetSpeed = std::clamp(forwardInput, -1.0f, 1.0f) * crawlSpeed;
        vault.crawlSpeed = MoveTowards(vault.crawlSpeed, targetSpeed, dt * (targetSpeed == 0.0f ? 7.0f : 4.9f));
        const float span = std::hypot(vault.exit.x - vault.entry.x, vault.exit.z - vault.entry.z);
        const float progress = vault.progress + vault.crawlSpeed * dt * (kCrawlEnd - kClimbEnd) / span;
        vault.returning = progress <= kClimbEnd && vault.crawlSpeed < 0.0f;
        vault.progress = std::clamp(progress, kClimbEnd, kCrawlEnd);
    } else {
        vault.progress = std::min(1.0f, vault.progress + dt / kAnimationSeconds);
        vault.active = vault.progress < 1.0f;
    }
}

XMFLOAT3 SampleWindowVault(const PlayerWindowVault& vault, float progress) {
    if (progress < kClimbEnd) {
        const float approach = SmoothStep(0.0f, kClimbEnd, progress);
        const float lift = SmoothStep(0.06f, kClimbEnd, progress);
        const float brace = MotionPulse(0.0f, 0.075f, 0.17f, progress);
        return {
            Lerp(vault.start.x, vault.entry.x, approach),
            Lerp(vault.start.y, vault.crossingEye, lift) - 0.055f * brace,
            Lerp(vault.start.z, vault.entry.z, approach)
        };
    }
    if (progress < kCrawlEnd) {
        const float blend = (progress - kClimbEnd) / (kCrawlEnd - kClimbEnd);
        const float duck = MotionPulse(kClimbEnd, 0.52f, kCrawlEnd, progress);
        const float sideways = 0.055f * vault.leanSide * duck;
        const float dx = vault.exit.x - vault.entry.x;
        const float dz = vault.exit.z - vault.entry.z;
        const float run = std::hypot(dx, dz);
        return {
            Lerp(vault.entry.x, vault.exit.x, blend) + dz * sideways / run,
            vault.crossingEye - 0.085f * duck,
            Lerp(vault.entry.z, vault.exit.z, blend) - dx * sideways / run
        };
    }
    const float settle = SmoothStep(kCrawlEnd, 1.0f, progress);
    const float drop = SmoothStep(kCrawlEnd, 0.90f, progress);
    const float impact = MotionPulse(0.85f, 0.91f, 1.0f, progress);
    return {
        Lerp(vault.exit.x, vault.landing.x, settle),
        Lerp(vault.crossingEye, vault.landing.y, drop) - 0.11f * impact,
        Lerp(vault.exit.z, vault.landing.z, settle)
    };
}

WindowVaultCameraPose SampleWindowVaultCamera(const PlayerWindowVault& vault, float progress) {
    const float brace = MotionPulse(0.0f, 0.087f, 0.22f, progress);
    const float lift = MotionPulse(0.10f, 0.27f, 0.48f, progress);
    const float duck = MotionPulse(0.24f, 0.52f, 0.77f, progress);
    const float drop = MotionPulse(0.72f, 0.87f, 0.96f, progress);
    const float rebound = MotionPulse(0.88f, 0.95f, 1.0f, progress);
    return {
        -0.055f * brace + 0.088f * lift - 0.09f * duck - 0.126f * drop + 0.043f * rebound,
        vault.leanSide * (0.035f * brace + 0.103f * duck - 0.042f * rebound)
    };
}
