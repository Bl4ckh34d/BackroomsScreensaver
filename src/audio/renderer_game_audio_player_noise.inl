// Game audio event dispatch, player noise, hearing radii, and short one-shot queues. 
// Included inside Renderer's private section from renderer_audio.inl.

    void RecomputePlayerNoiseRadiusFromPulses() {
        gameWorld_.RecomputePlayerNoiseRadiusFromPulses();
    }

    void UpdatePlayerAudibleSoundPulses(float dt) {
        gameWorld_.AdvancePlayerSoundPulses(dt);
    }

    void EmitPlayerAudibleSound(XMFLOAT3 pos, float radius, float life = 0.90f) {
        if (monsterPreview_.active || sessionRuntime_.mode == RendererRuntimeMode::MainMenu || radius <= 0.01f) return;
        pos.y = 0.08f;
        gameWorld_.EmitPlayerSoundPulse(pos, radius, life, 18);
    }

    void EmitPlayerAudibleSoundAtCamera(float radius, float life = 0.90f) {
        EmitPlayerAudibleSound({gameWorld_.player.position.x, 0.08f, gameWorld_.player.position.z}, radius, life);
    }
