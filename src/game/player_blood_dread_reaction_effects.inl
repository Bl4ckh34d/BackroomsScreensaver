            float closeness = Clamp01((point.radius - horizontalDist) / std::max(0.001f, point.radius));
            float gaze = SmoothStep(0.42f, 0.88f, aim);
            float reactionScale = std::clamp(point.dreadScale, 0.20f, 1.35f);
            float spike = std::max(settingsRuntime_.live.dreadJumpscareGain * 1.15f, 0.42f + closeness * 0.24f + gaze * 0.24f) * reactionScale;
            AddDread(spike);
            gameWorld_.QueueAudioEvent(GameAudioEvent::PlayerNoise(
                {world.playerPosition.x, 0.08f, world.playerPosition.z},
                JumpscareHearingRadius(0.92f),
                1.10f,
                GameAudioEventCategory::Scare));
            viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, (0.90f + closeness * 0.20f + gaze * 0.18f) * Lerp(0.62f, 1.0f, reactionScale));
            viewRuntime_.flashlightSnapCooldown = std::min(viewRuntime_.flashlightSnapCooldown, 0.08f);
            viewRuntime_.stumbleTimer = std::max(viewRuntime_.stumbleTimer, (0.10f + closeness * 0.08f) * reactionScale);
            viewRuntime_.stumbleDuration = std::max(viewRuntime_.stumbleDuration, 0.20f * reactionScale);
            viewRuntime_.secondsSinceLookBack = 0.0f;
        }
