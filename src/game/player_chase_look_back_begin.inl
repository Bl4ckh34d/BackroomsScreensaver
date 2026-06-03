    void BeginChaseLookBack(float urgency) {
        XMFLOAT3 focus = MonsterFocusPoint();
        viewRuntime_.chaseLookBackYaw = YawToPoint(focus);
        viewRuntime_.chaseLookBackPitch = std::clamp(PitchToPoint(focus), -0.34f, 0.24f);
        viewRuntime_.chaseLookBackDuration = RandRange(0.56f, 0.88f) * (1.0f - urgency * 0.10f);
        viewRuntime_.chaseLookBackTimer = viewRuntime_.chaseLookBackDuration;
        viewRuntime_.chaseLookBackCooldown = std::max(0.85f, RandRange(1.35f, 3.15f) * (1.0f - urgency * 0.24f));
        viewRuntime_.secondsSinceLookBack = 0.0f;
        viewRuntime_.stumbleTimer = std::min(viewRuntime_.stumbleTimer, 0.08f);
    }
