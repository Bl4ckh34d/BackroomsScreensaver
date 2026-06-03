template <typename MoveCallback>
inline PlayerManualMoveResult PlayerController::UpdateManualHorizontalMove(
    PlayerState& state,
    const PlayerManualCollisionMoveInput& input,
    MoveCallback moveCallback) {
    PlayerManualMoveInput moveInput{};
    moveInput.dt = input.dt;
    moveInput.wantsMove = input.wantsMove;
    moveInput.currentSpeed = state.smoothedMoveSpeed;
    moveInput.targetSpeed = input.targetSpeed;

    PlayerManualMoveResult result = UpdateManualMove(moveInput);
    state.smoothedMoveSpeed = result.smoothedSpeed;
    if (input.wantsMove && result.moveDistance > 0.0001f) {
        const float speed = std::max(0.1f, state.smoothedMoveSpeed);
        PlayerCollisionMoveRequest moveRequest{};
        moveRequest.stepX = input.moveDir.x * result.moveDistance;
        moveRequest.stepZ = input.moveDir.z * result.moveDistance;
        moveRequest.distance = result.moveDistance;
        moveRequest.speed = speed;
        result.movedSafely = moveCallback(moveRequest);
    }
    return result;
}
