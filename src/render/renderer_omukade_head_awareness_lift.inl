        float headAwareness = SmoothStep(0.18f, 1.0f, MonsterAwarenessAmount());
        if (headAwareness > 0.001f) {
            float targetY = std::clamp(playerPosition.y + 0.035f, 1.22f * modelY, settingsRuntime_.live.wallHeightMeters - 0.24f);
            float raise = (targetY - skull.y) * (0.62f * headAwareness);
            skull.y += raise;
            headRoot.y += raise * 0.82f;
        }
