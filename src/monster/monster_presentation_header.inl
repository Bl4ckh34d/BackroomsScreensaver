// Monster runtime presentation state helpers. 
// Included inside Renderer's private section from monster_ai.inl.

    float MonsterHeadBobOffset() const {
        float phase = monsterPreview_.active ? timeRuntime_.time * 2.35f : monsterPresentation_.headBobPhase;
        float chase = monsterPreview_.active ? 0.0f : monsterPresentation_.headChaseBlend;
        float amplitude = Lerp(0.050f, 0.086f, chase);
        float secondary = Lerp(0.006f, 0.014f, chase);
        return std::sin(phase) * amplitude + std::sin(phase * 2.0f + 0.65f) * secondary;
    }
