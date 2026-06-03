    // Host/window lifetime and frame timing.
    RendererHostRuntimeState hostRuntime_{};
    RendererTimeRuntimeState timeRuntime_{};
    RendererDebugRuntimeState debugRuntime_{};
    StartupProgressRuntimeState startupRuntime_{};

    // GPU resources, render targets, shaders, buffers, and pipeline objects.
    RendererDeviceRuntimeState d3dRuntime_{};
    RenderTargetRuntimeState renderTargetRuntime_{};
    ShaderRuntimeState shaders_{};
    InputLayoutRuntimeState inputLayouts_{};
    RenderBufferRuntimeState renderBuffers_{};
    MaterialTextureRuntimeState materialTextures_{};
    RuntimeTextureResourceState runtimeTextures_{};
    ShadowResourceRuntimeState shadowResources_{};
    PipelineStateRuntimeState pipelineStates_{};
    GpuProfileRuntimeState gpuProfileRuntime_{};
    PresentRuntimeState presentRuntime_{};

    // Render caches, generated geometry, and renderer-owned assets.
    StaticSceneGeometryRuntimeState staticSceneGeometry_{};
    MapOverlayRuntimeState mapOverlayRuntime_{};
    DynamicGeometryRuntimeState dynamicGeometry_{};
    RenderAssetRuntimeState renderAssets_{};

    // Session/gameplay state that remains renderer-hosted while GameWorld moves out.
    GameSessionRuntimeState sessionRuntime_{};
    MainMenuRuntimeState menuRuntime_{};
    GameWorld gameWorld_{};
    BenchmarkRuntimeState benchmarkRuntime_{};

    // HUD, overlays, menu presentation, and other screen-space presentation state.
    HudNotificationRuntimeState hudNotification_{};
    FixtureShadowRuntimeState fixtureShadowRuntime_{};
    MonsterPreviewRuntimeState monsterPreview_{};

    // Runtime settings, audio bridge, player view helpers, and scare/effect state.
    RendererSettingsRuntimeState settingsRuntime_{};
    RendererAudioRuntimeState audioRuntime_{};
    PlayerCameraRuntimeState cameraRuntime_{};
    EnvironmentalEffectRuntimeState effectRuntime_{};
    ScareEffectRuntimeState scareRuntime_{};
    PlayerViewRuntimeState viewRuntime_{};
    ExitDoorPresentationState exitDoorPresentation_{};

    // Monster presentation state; monster decisions live in GameWorld/MonsterState.
    MonsterPresentationRuntimeState monsterPresentation_{};
