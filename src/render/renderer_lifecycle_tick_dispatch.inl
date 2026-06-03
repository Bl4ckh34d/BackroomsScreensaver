    void Tick() {
        ULONGLONG now = GetTickCount64();
        float dt = std::min(0.05f, static_cast<float>(now - timeRuntime_.lastTicks) / 1000.0f);
        timeRuntime_.lastTicks = now;
        TickFrame(dt);
    }

    void TickFixed(float dt) {
        timeRuntime_.lastTicks = GetTickCount64();
        TickFrame(std::clamp(dt, 0.0f, 0.05f));
    }
