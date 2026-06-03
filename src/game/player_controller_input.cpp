#include "../platform/platform_headers.h"

#include "../core/math_utils.h"
#include "player_state.h"
#include "player_controller.h"
PlayerFlashlightInputResult PlayerController::UpdateFlashlightInput(
    PlayerState& state,
    bool inputEnabled,
    bool flashlightPressed) {
    PlayerFlashlightInputResult result{};
    if (!inputEnabled) {
        state.flashlightPressedLastFrame = false;
        result.enabled = state.flashlightEnabled;
        return result;
    }

    if (flashlightPressed && !state.flashlightPressedLastFrame) {
        state.flashlightEnabled = !state.flashlightEnabled;
        result.toggled = true;
    }
    state.flashlightPressedLastFrame = flashlightPressed;
    result.enabled = state.flashlightEnabled;
    return result;
}

bool PlayerController::ConsumeInteractPress(PlayerState& state, bool interactPressed) {
    bool pressedNow = interactPressed && !state.interactPressedLastFrame;
    state.interactPressedLastFrame = interactPressed;
    return pressedNow;
}

void PlayerController::ResetInputLatches(PlayerState& state, bool flashlightEnabled) {
    state.flashlightEnabled = flashlightEnabled;
    state.flashlightPressedLastFrame = false;
    state.interactPressedLastFrame = false;
}
