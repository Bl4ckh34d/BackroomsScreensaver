    void DrainGameAudioEvents(bool playbackEnabled = true) {
        std::vector<GameAudioEvent> events = gameWorld_.DrainAudioEvents();
        for (const GameAudioEvent& event : events) {
            DispatchGameAudioEvent(event, playbackEnabled);
        }
    }
