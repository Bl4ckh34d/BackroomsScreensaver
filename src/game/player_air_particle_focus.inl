    void UpdateAirParticleFocus(float dt) {
        float target = FlashlightFocusTargetDistance();
        if (viewRuntime_.airFocusDistance <= 0.0f) viewRuntime_.airFocusDistance = target;
        float follow = std::min(1.0f, std::max(0.001f, dt) * 1.85f);
        viewRuntime_.airFocusDistance += (target - viewRuntime_.airFocusDistance) * follow;
    }
