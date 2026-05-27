// Common scalar math, hashing, and procedural noise helpers.
// Included from main.cpp after shared render/runtime types.

bool operator==(Tile a, Tile b) {
    return a.x == b.x && a.y == b.y;
}

float Clamp01(float v) {
    return std::clamp(v, 0.0f, 1.0f);
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float MoveTowards(float current, float target, float maxDelta) {
    if (maxDelta <= 0.0f) return current;
    float delta = target - current;
    if (std::abs(delta) <= maxDelta) return target;
    return current + (delta > 0.0f ? maxDelta : -maxDelta);
}

float AngleWrap(float a) {
    while (a > kPi) a -= kPi * 2.0f;
    while (a < -kPi) a += kPi * 2.0f;
    return a;
}

float SmoothStep(float a, float b, float x) {
    float t = Clamp01((x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
}

uint32_t Hash2(int x, int y, uint32_t seed) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

uint32_t Hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u ^
        static_cast<uint32_t>(y) * 668265263u ^
        static_cast<uint32_t>(z) * 2246822519u ^
        seed * 3266489917u;
    h = (h ^ (h >> 16)) * 2246822519u;
    h = (h ^ (h >> 13)) * 3266489917u;
    return h ^ (h >> 16);
}

float Rand01(int x, int y, uint32_t seed) {
    return (Hash2(x, y, seed) & 0x00ffffffu) / 16777215.0f;
}

float Rand01(int x, int y, int z, uint32_t seed) {
    return (Hash3(x, y, z, seed) & 0x00ffffffu) / 16777215.0f;
}

float ValueNoise(float x, float y, uint32_t seed) {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - static_cast<float>(ix);
    float fy = y - static_cast<float>(iy);
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    float a = Rand01(ix, iy, seed);
    float b = Rand01(ix + 1, iy, seed);
    float c = Rand01(ix, iy + 1, seed);
    float d = Rand01(ix + 1, iy + 1, seed);
    return Lerp(Lerp(a, b, fx), Lerp(c, d, fx), fy);
}

float FractalNoise(float x, float y, uint32_t seed) {
    float sum = 0.0f;
    float amp = 0.55f;
    float freq = 1.0f;
    float norm = 0.0f;
    for (int i = 0; i < 5; ++i) {
        sum += ValueNoise(x * freq, y * freq, seed + i * 97u) * amp;
        norm += amp;
        amp *= 0.5f;
        freq *= 2.13f;
    }
    return sum / norm;
}

uint8_t Byte(float v) {
    return static_cast<uint8_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
}
