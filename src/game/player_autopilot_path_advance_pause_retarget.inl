            softStopActive = !panicActive && (cameraRuntime_.stopTimer > 0.0f || bloodFocusActive || ventReactionActive);
            if (pauseStarted || softStopActive) {
                if (pauseStarted && cameraRuntime_.junctionScanActive) {
                    return;
                }
