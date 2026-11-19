    void ContinueAfterScoreScreen() {
        if (!gameWorld_.CanContinueAfterScoreScreen()) return;
        PrepareNextPlayableMaze();
        if (nextMazePreparation_) {
            nextMazePreparation_->continueRequested = true;
            if (nextMazePreparation_->scene.valid() &&
                nextMazePreparation_->scene.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
        }
        viewRuntime_.exitScoreContinueReady = false;
        viewRuntime_.scoreContinuePressedLastFrame = sessionRuntime_.input.anyKey;
        gameWorld_.EndExitTransition();
        auto next = std::move(nextMazePreparation_);
        if (!next) {
            BeginPlayableLevel(gameWorld_.NextLevelAfterScoreScreen());
            return;
        }
        std::unique_ptr<Renderer> prepared;
        if (next->scene.valid()) {
            try {
                prepared = next->scene.get();
            } catch (const std::exception&) {
                StartupProfileLine(L"Background maze preparation failed");
            }
        }
        gameWorld_.BeginPlayableLevel(next->level);
        ApplyPlayableLevelSpec(next->level, next->flickerRoll);
        if (prepared) {
            ActivatePreparedMaze(*prepared);
        } else {
            RestartMaze();
        }
        ShowPlayableLevelStartNotice();
    }

    void UpdatePlayableProgressionTimers(float dt) {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame) return;
        gameWorld_.AdvancePlayableProgressionTimers(dt);
    }
