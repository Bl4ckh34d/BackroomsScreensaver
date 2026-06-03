    bool initialized_ = false;
    bool comInitialized_ = false;
    HRESULT comHr_ = E_FAIL;
    Microsoft::WRL::ComPtr<IXAudio2> xaudio_;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;
    UINT32 outputChannels_ = 2;
    X3DAUDIO_HANDLE x3dHandle_{};
    X3DAUDIO_LISTENER listener_{};
    std::vector<AudioSample> samples_;
    std::vector<std::vector<size_t>> groups_;
    std::vector<AudioVoiceInstance> voices_;
    std::mt19937 rng_{0xA7710u};
    bool muted_ = false;
    float masterVolume_ = 1.0f;
    float effectsVolume_ = 1.0f;
    float ambienceVolume_ = 1.0f;
    float monsterVolume_ = 1.0f;

