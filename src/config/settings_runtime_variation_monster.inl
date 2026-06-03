    s.monsterScale = JitterScaled(seed, 601, s.monsterScale, v, 0.14f, 0.25f, 4.0f);
    s.monsterSpeed = JitterScaled(seed, 602, s.monsterSpeed, v, 0.22f, 0.1f, 4.0f);
    s.monsterSprintSpeed = JitterScaled(seed, 603, s.monsterSprintSpeed, v, 0.22f, 0.1f, 4.0f);
    s.monsterKillDistance = JitterScaled(seed, 604, s.monsterKillDistance, v, 0.10f, 0.2f, 4.0f);
    s.monsterVisibleDistance = JitterScaled(seed, 605, s.monsterVisibleDistance, v, 0.25f, 1.0f, 60.0f);
