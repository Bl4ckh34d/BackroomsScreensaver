float RuntimeSigned(uint32_t seed, int salt) {
    return Rand01(salt, salt * 37 + 19, seed ^ 0x6d2b79f5u) * 2.0f - 1.0f;
}

float JitterScaled(uint32_t seed, int salt, float value, float amount, float maxFraction, float minValue, float maxValue) {
    if (amount <= 0.0001f || std::abs(value) <= 0.0001f) return std::clamp(value, minValue, maxValue);
    float scale = 1.0f + RuntimeSigned(seed, salt) * amount * maxFraction;
    return std::clamp(value * scale, minValue, maxValue);
}

int JitterInt(uint32_t seed, int salt, int value, float amount, float maxFraction, int minValue, int maxValue) {
    if (value == 0) return std::clamp(value, minValue, maxValue);
    float jittered = JitterScaled(seed, salt, static_cast<float>(value), amount, maxFraction,
        static_cast<float>(minValue), static_cast<float>(maxValue));
    return std::clamp(static_cast<int>(std::round(jittered)), minValue, maxValue);
}

int JitterIntRange(uint32_t seed, int salt, int value, int range, float amount, int minValue, int maxValue) {
    if (amount <= 0.0001f || range <= 0) return std::clamp(value, minValue, maxValue);
    float delta = RuntimeSigned(seed, salt) * amount * static_cast<float>(range);
    return std::clamp(value + static_cast<int>(std::round(delta)), minValue, maxValue);
}
