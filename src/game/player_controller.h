#pragma once

#include "game_input.h"
#include "player_state.h"
#include "player_controller_stamina_types.h"
#include "player_controller_move_types.h"
#include "player_controller_motion_types.h"
#include "player_controller_posture_types.h"
#include "player_controller_look_types.h"
#include "player_controller_input_types.h"
#include "player_controller_manual_types.h"

class PlayerController {
public:
    static PlayerMoveIntentResult BuildMoveIntent(const PlayerMoveIntentInput& input);
    static PlayerManualMoveResult UpdateManualMove(const PlayerManualMoveInput& input);

    template <typename MoveCallback>
    static PlayerManualMoveResult UpdateManualHorizontalMove(
        PlayerState& state,
        const PlayerManualCollisionMoveInput& input,
        MoveCallback moveCallback);

    static PlayerManualControlResult UpdateManualControl(
        PlayerState& state,
        const PlayerManualControlInput& input,
        const PlayerManualControlServices& services);

    static PlayerRunMotionResult UpdateRunMotion(const PlayerRunMotionInput& input);
    static void UpdateManualCameraHeight(PlayerState& state, const PlayerManualCameraHeightInput& input);
    static void AdvanceStepPhase(PlayerState& state, float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning);
    static PlayerManualLookResult UpdateManualLook(const PlayerManualLookInput& input);
    static PlayerCrouchResult UpdateCrouch(PlayerState& state, const PlayerCrouchInput& input);
    static PlayerFlashlightInputResult UpdateFlashlightInput(PlayerState& state, bool inputEnabled, bool flashlightPressed);
    static bool ConsumeInteractPress(PlayerState& state, bool interactPressed);
    static void ResetInputLatches(PlayerState& state, bool flashlightEnabled);
    static PlayerStaminaResult UpdateStamina(PlayerState& state, const PlayerStaminaInput& input);
};

#include "player_controller.inl"