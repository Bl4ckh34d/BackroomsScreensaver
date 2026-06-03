// Liquid placement shared water helpers.

    static float WaterDecalMaterial(float seed, float bandStart, float bandWidth) {
        float h = std::fmod(std::abs(seed) * 37.719f + 0.137f, 1.0f);
        float safeWidth = std::max(0.0f, std::min(bandWidth, 0.043f - bandStart));
        return 11.006f + bandStart + h * safeWidth;
    }
