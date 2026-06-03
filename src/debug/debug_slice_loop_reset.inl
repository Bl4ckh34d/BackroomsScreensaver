    void ResetDebugSliceLoopState() {
        debugRuntime_.debugSliceLoopCycle = -1;
        effectRuntime_.sparks.clear();
        effectRuntime_.sparkFlashes.clear();
        effectRuntime_.sparkChains.clear();
        effectRuntime_.steam.clear();
        effectRuntime_.ventDrops.clear();
        for (SparkEmitter& emitter : effectRuntime_.sparkEmitters) {
            emitter.triggered = false;
        }
        for (SteamEmitter& emitter : effectRuntime_.steamEmitters) {
            emitter.triggered = false;
            emitter.panelDropped = false;
        }
    }
