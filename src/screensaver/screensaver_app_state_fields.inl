    bool loadingWarmupPending = false;
    ULONGLONG loadingWarmupStart = 0;
    int loadingWarmupAttempts = 0;
    bool quitting = false;
    bool firstMouse = true;
    POINT initialMouse{};

    struct CloneOutput {
        HWND hwnd = nullptr;
        HWND loadingOverlay = nullptr;
        Renderer renderer;
        bool loadingWarmupPending = false;
        ULONGLONG loadingWarmupStart = 0;
        int loadingWarmupAttempts = 0;
    };
    std::vector<std::unique_ptr<CloneOutput>> clones;
