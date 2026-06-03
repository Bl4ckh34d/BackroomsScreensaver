    void UpdateChaseLookBackTarget(float dt) {
        if (viewRuntime_.chaseLookBackTimer <= 0.0f || viewRuntime_.chaseLookBackDuration <= 0.001f) return;
        if (!MonsterCanSeePlayer()) return;
        XMFLOAT3 focus = MonsterFocusPoint();
        float liveYaw = YawToPoint(focus);
        float livePitch = std::clamp(PitchToPoint(focus), -0.34f, 0.24f);
        float follow = std::min(1.0f, dt * 2.35f);
        viewRuntime_.chaseLookBackYaw += AngleWrap(liveYaw - viewRuntime_.chaseLookBackYaw) * follow;
        viewRuntime_.chaseLookBackPitch += (livePitch - viewRuntime_.chaseLookBackPitch) * follow;
    }
