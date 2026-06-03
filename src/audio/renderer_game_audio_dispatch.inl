    void DispatchGameAudioEvent(const GameAudioEvent& event, bool playbackEnabled = true) {
        if (event.kind == GameAudioEventKind::PlayerNoise) {
            EmitPlayerAudibleSound(event.pos, event.hearingRadius, event.hearingLife);
            return;
        }

        if (playbackEnabled && event.volume > 0.0f) {
            float occlusion = event.useOcclusion ? AudioOcclusionFor(event.pos) : 0.0f;
            occlusion = std::min(occlusion, event.occlusionLimit);
            audioRuntime_.engine.PlayRandom(event.sound, event.bus, event.pos, event.volume, event.spatial, occlusion);
        }
        if (event.hearingRadius > 0.0f) {
            EmitPlayerAudibleSound(event.pos, event.hearingRadius, event.hearingLife);
        }
    }
