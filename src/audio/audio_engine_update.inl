    template <typename OcclusionFn>
    void Update(float dt, OcclusionFn occlusionForPosition) {
        if (!initialized_) return;
        dt = std::max(0.0f, dt);
        for (size_t i = 0; i < voices_.size();) {
            AudioVoiceInstance& instance = voices_[i];
            XAUDIO2_VOICE_STATE state{};
            instance.voice->GetState(&state);
            if (!instance.loop && state.BuffersQueued == 0) {
                instance.voice->DestroyVoice();
                voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
                continue;
            }
            float distanceGain = SpatialDistanceGain(instance);
            if (instance.spatial && distanceGain <= 0.0001f) {
                instance.voice->SetVolume(0.0f);
                instance.occlusionRefresh = std::min(instance.occlusionRefresh, 0.10f);
                ++i;
                continue;
            }
            if (!instance.spatial || instance.bus == AudioBus::Music) {
                instance.occlusion = 0.0f;
                instance.occlusionRefresh = 0.0f;
            } else {
                instance.occlusionRefresh -= dt;
            }
            if (instance.spatial && instance.bus != AudioBus::Music && instance.occlusionRefresh <= 0.0f) {
                instance.occlusion = std::clamp(occlusionForPosition(instance.pos), 0.0f, 8.0f);
                instance.occlusionRefresh = instance.bus == AudioBus::Ambience ? 0.32f : 0.14f;
            }
            Apply3D(instance);
            ++i;
        }
    }

    template <typename OcclusionFn>
    void Update(OcclusionFn occlusionForPosition) {
        Update(0.0f, occlusionForPosition);
    }

    void Update() {
        Update(0.0f, [](XMFLOAT3) { return 0.0f; });
    }
