
    float v = Clamp01(s.runVariation);
    if (v <= 0.0001f) return;
    constexpr float kUserFraction = 0.15f;

    s.roomCount = JitterIntRange(seed, 101, s.roomCount, s.roomCountRange, v, 0, 80);
    s.roomMinRadius = JitterIntRange(seed, 102, s.roomMinRadius, s.roomMinRadiusRange, v, 1, 12);
    s.roomMaxRadius = JitterIntRange(seed, 103, s.roomMaxRadius, s.roomMaxRadiusRange, v, s.roomMinRadius, 16);
