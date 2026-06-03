        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && gameWorld_.PlayableScoreScreenActive()) {
            pushRect(0.0f, 0.0f, static_cast<float>(hostRuntime_.width), static_cast<float>(hostRuntime_.height), {0.0f, 0.0f, 0.0f, 0.34f});
        }

        float x = 24.0f;
        float y = static_cast<float>(hostRuntime_.height) - 58.0f;
        float w = std::clamp(static_cast<float>(hostRuntime_.width) * 0.22f, 180.0f, 290.0f);
        pushBar(x, y, w, 14.0f, world.playerHealth / 100.0f, {0.72f, 0.05f, 0.045f, 0.92f});
        float staminaFill = world.playerStamina / 100.0f;
        float fatigue = 1.0f - SmoothStep(0.06f, 0.34f, staminaFill);
        XMFLOAT4 staminaColor{
            Lerp(0.88f, 0.92f, fatigue),
            Lerp(0.72f, 0.36f, fatigue),
            Lerp(0.23f, 0.08f, fatigue),
            0.88f
        };
        pushBar(x, y + 24.0f, w, 10.0f, staminaFill, staminaColor);
