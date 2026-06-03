        cb.fog0 = {
            settingsRuntime_.live.fogStartMeters,
            settingsRuntime_.live.fogEndMeters,
            settingsRuntime_.live.fogDarkness,
            0.0f
        };
        if (monsterPreview_.active) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        if (gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        cb.ao0 = {
            settingsRuntime_.live.cornerAOIntensity,
            settingsRuntime_.live.cornerAORadius,
            settingsRuntime_.live.floorCeilingAOIntensity,
            tileAverage
        };
