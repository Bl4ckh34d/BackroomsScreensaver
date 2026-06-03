#pragma once

enum class MonsterGameplayEventKind {
    KillPlayer
};

struct MonsterUpdateInput {
    float dt = 0.0f;
    float time = 0.0f;
    const Settings* settings = nullptr;
    const Maze* maze = nullptr;
    const PlayerState* player = nullptr;
    std::vector<PlayerAudibleSoundPulse>* playerSoundPulses = nullptr;
    bool ignorePlayer = false;
    bool preview = false;
};

struct MonsterUpdateOutput {
    bool seesPlayer = false;
    bool heardPlayer = false;
    bool hasGoal = false;
    bool moved = false;
    float distanceToPlayer = 0.0f;
    std::array<MonsterGameplayEventKind, 4> gameplayEvents{};
    size_t gameplayEventCount = 0;

    void AddGameplayEvent(MonsterGameplayEventKind kind) {
        if (gameplayEventCount >= gameplayEvents.size()) return;
        gameplayEvents[gameplayEventCount++] = kind;
    }

    bool HasGameplayEvent(MonsterGameplayEventKind kind) const {
        for (size_t i = 0; i < gameplayEventCount; ++i) {
            if (gameplayEvents[i] == kind) return true;
        }
        return false;
    }
};

inline float MonsterSightDistanceMeters(const Settings& settings) {
    float visibleDistance = std::max(0.1f, settings.monsterVisibleDistance);
    if (settings.fogDarkness > 0.02f) {
        visibleDistance = std::min(visibleDistance, std::max(0.1f, settings.fogEndMeters));
    }
    return visibleDistance;
}

inline int MonsterSoundWallBlocksBetween(const Maze& maze, XMFLOAT3 from, XMFLOAT3 to) {
    Tile fromTile = maze.TileFromWorld(from.x, from.z);
    Tile toTile = maze.TileFromWorld(to.x, to.z);
    if (!maze.InBounds(fromTile.x, fromTile.y) || !maze.InBounds(toTile.x, toTile.y)) return 4;
    if (fromTile == toTile) return maze.IsOpen(fromTile.x, fromTile.y) ? 0 : 1;
    float dx = to.x - from.x;
    float dz = to.z - from.z;
    float dist = std::sqrt(dx * dx + dz * dz);
    int steps = std::clamp(static_cast<int>(dist / std::max(0.05f, maze.TileMinimum() * 0.12f)), 6, 160);
    int blocks = 0;
    Tile previous{-100000, -100000};
    for (int i = 1; i <= steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        Tile sample = maze.TileFromWorld(from.x + dx * t, from.z + dz * t);
        if (!maze.InBounds(sample.x, sample.y)) {
            ++blocks;
            if (blocks >= 8) return blocks;
            continue;
        }
        if (!maze.IsOpen(sample.x, sample.y) && !(sample == previous)) {
            ++blocks;
            if (blocks >= 8) return blocks;
        }
        previous = sample;
    }
    return blocks;
}
