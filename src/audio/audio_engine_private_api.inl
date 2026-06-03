    static constexpr size_t kInvalidSample = static_cast<size_t>(-1);
    static constexpr uint32_t FourCC(char a, char b, char c, char d) {
        return static_cast<uint32_t>(static_cast<uint8_t>(a)) |
            (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 8) |
            (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 16) |
            (static_cast<uint32_t>(static_cast<uint8_t>(d)) << 24);
    }

    std::filesystem::path ResolveSoundFolder(const AudioEngineAssets& assets, const std::wstring& folder) const;
    void AddFolder(const AudioEngineAssets& assets, GameSound sound, const std::wstring& folder,
                   const std::wstring& pattern = L"*.wav");
    bool LoadWav(const std::filesystem::path& path, AudioSample& out) const;
    size_t PickSample(GameSound sound);
    float BusVolume(AudioBus bus) const;
    void StartVoice(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume, bool loop, bool spatial,
                    float frequencyRatio = 1.0f, int tag = -1, float initialOcclusion = 0.0f,
                    AudioToneProfile toneProfile = AudioToneProfile::Normal);
    void Apply3D(AudioVoiceInstance& instance);
    float SpatialDistanceGain(const AudioVoiceInstance& instance) const;
    float OcclusionGain(AudioBus bus, float occlusion) const;
