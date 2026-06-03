    float ChaseLookBackWeight() const {
        if (viewRuntime_.chaseLookBackTimer <= 0.0f || viewRuntime_.chaseLookBackDuration <= 0.001f) return 0.0f;
        float t = 1.0f - viewRuntime_.chaseLookBackTimer / viewRuntime_.chaseLookBackDuration;
        return SmoothStep(0.0f, 0.22f, t) * (1.0f - SmoothStep(0.70f, 1.0f, t));
    }
