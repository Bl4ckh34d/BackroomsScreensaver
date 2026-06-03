    void ReconcileCollectiblePageRestore(int savedCollectedPages);
    void RecomputePlayerNoiseRadiusFromPulses();
    void AdvancePlayerSoundPulses(float dt);
    bool EmitPlayerSoundPulse(const XMFLOAT3& pos, float radius, float life, size_t maxPulses);
    void QueueAudioEvent(const GameAudioEvent& event);
    std::vector<GameAudioEvent> DrainAudioEvents();
