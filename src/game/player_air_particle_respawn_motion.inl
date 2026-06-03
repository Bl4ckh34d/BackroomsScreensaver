        p.pos = pos;
        float driftRoll = std::pow(RandRange(0.0f, 1.0f), 1.35f);
        float driftScale = Lerp(0.36f, 2.15f, driftRoll);
        if (RandRange(0.0f, 1.0f) < 0.075f) driftScale *= RandRange(1.35f, 2.25f);
        if (p.nearLayer > 0.5f) driftScale *= RandRange(0.78f, 1.32f);
        p.vel = {
            RandRange(-0.030f, 0.030f) * driftScale,
            RandRange(-0.010f, 0.026f) * driftScale,
            RandRange(-0.030f, 0.030f) * driftScale
        };
        p.life = RandRange(28.0f, 68.0f);
        p.age = initial ? RandRange(0.0f, p.life * 0.82f) : 0.0f;
