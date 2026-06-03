#pragma once

#include "player_state.h"

struct PlayerMoveIntentInput {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float yaw = 0.0f;
};

struct PlayerMoveIntentResult {
    bool wantsMove = false;
    XMFLOAT3 moveDir{0.0f, 0.0f, 1.0f};
};

struct PlayerManualMoveInput {
    float dt = 0.0f;
    bool wantsMove = false;
    float currentSpeed = 0.0f;
    float targetSpeed = 0.0f;
};

struct PlayerManualMoveResult {
    float smoothedSpeed = 0.0f;
    float moveDistance = 0.0f;
    bool movedSafely = false;
};

struct PlayerCollisionMoveRequest {
    float stepX = 0.0f;
    float stepZ = 0.0f;
    float distance = 0.0f;
    float speed = 0.0f;
};

struct PlayerManualCollisionMoveInput {
    float dt = 0.0f;
    bool wantsMove = false;
    float targetSpeed = 0.0f;
    XMFLOAT3 moveDir{0.0f, 0.0f, 1.0f};
};
