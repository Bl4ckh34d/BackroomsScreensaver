#include "../platform/platform_headers.h"

#include "../core/math_utils.h"
#include "player_state.h"
#include "player_controller.h"
PlayerStaminaResult PlayerController::UpdateStamina(PlayerState& state, const PlayerStaminaInput& input) {
    PlayerStaminaResult result{};
    result.walkSpeed = input.walkSpeed;
    result.sprintSpeed = std::max(input.runSpeed, input.walkSpeed * 1.35f);

    if (input.infiniteStamina) {
        state.stamina = 100.0f;
        state.staminaRegenDelay = 0.0f;
        state.sprintStaminaLocked = false;
    }

    constexpr float kJogStamina = 100.0f / 3.0f;
    constexpr float kExhaustedStamina = 5.0f;
    if (state.stamina <= 0.001f) state.sprintStaminaLocked = true;
    if (state.sprintStaminaLocked && state.stamina >= kExhaustedStamina) state.sprintStaminaLocked = false;

    const bool sprintAllowed = input.infiniteStamina ||
        (!state.sprintStaminaLocked && state.stamina > 0.001f);
    result.wantsSprint = input.sprintPressed && input.wantsMove && !input.crouching && sprintAllowed;
    result.staminaJog = result.wantsSprint && !input.infiniteStamina && state.stamina <= kJogStamina;

    const float jogSpeed = input.walkSpeed + (result.sprintSpeed - input.walkSpeed) * 0.55f;
    result.targetSpeed = result.wantsSprint
        ? (result.staminaJog ? jogSpeed : result.sprintSpeed)
        : input.walkSpeed;
    result.targetSpeed *= input.crouchSpeedScale;

    if (result.wantsSprint && !input.infiniteStamina) {
        const float drainRate = state.stamina <= kExhaustedStamina
            ? (kExhaustedStamina / 30.0f)
            : (result.staminaJog ? 2.00f : 5.00f);
        state.stamina = std::max(0.0f, state.stamina - drainRate * input.dt);
        state.staminaRegenDelay = 0.85f;
        if (state.stamina <= 0.001f) {
            result.wantsSprint = false;
            state.sprintStaminaLocked = true;
            result.targetSpeed = input.walkSpeed;
        }
    } else if (result.wantsSprint && input.infiniteStamina) {
        state.stamina = 100.0f;
        state.staminaRegenDelay = 0.0f;
    } else {
        if (state.stamina < kExhaustedStamina) {
            state.staminaRegenDelay = 0.0f;
            state.stamina = std::min(100.0f, state.stamina + (kExhaustedStamina / 3.0f) * input.dt);
        } else {
            state.staminaRegenDelay = std::max(0.0f, state.staminaRegenDelay - input.dt);
            if (state.staminaRegenDelay <= 0.0f) {
                const float regenRate = state.stamina < kJogStamina ? 10.50f : 5.20f;
                state.stamina = std::min(100.0f, state.stamina + regenRate * input.dt);
            }
        }
    }

    return result;
}
