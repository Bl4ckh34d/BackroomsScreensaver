    void StopAll();
    void PlayRandom(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                    float initialOcclusion = 0.0f);
    void StartLoop(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                   float initialOcclusion = 0.0f);
    void StartLoopTagged(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial, int tag,
                         float initialOcclusion = 0.0f);
    void StartLoopTaggedSample(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume,
                               bool spatial, int tag, float initialOcclusion = 0.0f);
    bool HasTaggedVoice(int tag) const;
    void StopTaggedVoice(int tag);
    size_t PickRandomSample(GameSound sound);
    size_t PickStableSample(GameSound sound, uint32_t stableId) const;
    size_t PickRandomSampleExcept(GameSound sound, size_t excludedSampleIndex);
    void PlaySample(size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                    float frequencyRatio = 1.0f, GameSound sound = GameSound::MonsterGrowl,
                    float initialOcclusion = 0.0f, AudioToneProfile toneProfile = AudioToneProfile::Normal);
