    struct PlayableSnapshot {
        Settings settings;
        GameWorldSnapshotState gameWorld;
        uint32_t runtimeSeed = 1;
        std::mt19937 rng;
        PlayerCameraRuntimeState cameraRuntime;
        EnvironmentalEffectRuntimeState effectRuntime;
        ScareEffectRuntimeState scareRuntime;
        PlayerViewRuntimeState viewRuntime;
        float exitDoorAngle = 0.0f;
        MonsterPresentationSnapshotState monsterPresentation;
    };

    std::unique_ptr<PlayableSnapshot> pausedPlayableSnapshot_;

    void SavePlayableSnapshot() {
        auto s = std::make_unique<PlayableSnapshot>();
        s->settings = settingsRuntime_.live;
        s->gameWorld = gameWorld_.CaptureSnapshotState();
        s->runtimeSeed = sessionRuntime_.runtimeSeed;
        s->rng = sessionRuntime_.rng;
        s->cameraRuntime = cameraRuntime_;
        s->effectRuntime = effectRuntime_;
        s->scareRuntime = scareRuntime_;
        s->viewRuntime = viewRuntime_;
        s->exitDoorAngle = exitDoorPresentation_.angle;
        s->monsterPresentation = monsterPresentation_.CaptureSnapshot();
        pausedPlayableSnapshot_ = std::move(s);
    }

    void RestorePlayableSnapshot() {
        PlayableSnapshot s = std::move(*pausedPlayableSnapshot_);
        sessionRuntime_.ConfigurePlayableManual();
        gameWorld_.progressionEnabled = true;
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        EnsureFullSceneAssets();
        settingsRuntime_.live = s.settings;
        gameWorld_.RestoreSnapshotGenerationState(s.gameWorld);
        sessionRuntime_.runtimeSeed = s.runtimeSeed;
        sessionRuntime_.rng = s.rng;
        CreateMazeMaskTexture();
        CreateMazeMesh();
        SetupPersistentAudioEmitters();
        gameWorld_.RestoreSnapshotRuntimeState(std::move(s.gameWorld));
        cameraRuntime_ = std::move(s.cameraRuntime);
        effectRuntime_ = std::move(s.effectRuntime);
        scareRuntime_ = std::move(s.scareRuntime);
        effectRuntime_.lampDamageDirty = true;
        viewRuntime_ = std::move(s.viewRuntime);
        exitDoorPresentation_.angle = s.exitDoorAngle;
        monsterPresentation_.RestoreSnapshot(std::move(s.monsterPresentation));
        sessionRuntime_.ClearInput();
        InvalidateRect(hostRuntime_.hwnd, nullptr, FALSE);
    }
