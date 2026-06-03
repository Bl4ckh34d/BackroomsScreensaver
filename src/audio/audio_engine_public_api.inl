    bool Initialize(const AudioEngineSettings& settings);
    void Shutdown();
    void ApplySettings(const AudioEngineSettings& settings);
    void LoadAll(const AudioEngineAssets& assets);
    void SetListener(XMFLOAT3 pos, XMFLOAT3 forward);
