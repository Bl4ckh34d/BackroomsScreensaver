        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            viewRuntime_.fadeInTimer = 0.0f;
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
        } else if (gBloodDebugEveryWall) {
            ApplyBloodDebugCamera();
            viewRuntime_.fadeInTimer = 0.0f;
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
        }
        if (settingsRuntime_.live.bloodStudyView) {
            ApplyBloodStudyCamera();
            viewRuntime_.fadeInTimer = 0.0f;
        }
        if (BenchmarkDemoEnabled()) {
            benchmarkRuntime_.active = true;
            benchmarkRuntime_.timer = 0.0f;
            ApplyBenchmarkDemoCamera(0.0f);
            viewRuntime_.fadeInTimer = 0.0f;
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
        }
        if (monsterPreview_.active) {
            gameWorld_.SetMonsterPreviewPose();
            ResetMonsterPresentationState(true, true);
            PrimeMonsterTrail(0.10f);
            SetMonsterPreviewCamera(0.0f);
            viewRuntime_.dangerLevel = 0.45f;
            viewRuntime_.dreadLevel = 0.35f;
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
        }
