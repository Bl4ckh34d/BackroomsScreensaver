
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return;
        if (menuRuntime_.startTransitionComplete) return;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
