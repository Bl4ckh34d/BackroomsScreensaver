    static float LiquidCanvasMaterial(bool water, float seed) {
        float h = std::fmod(std::abs(seed) * 37.719f + 0.137f, 1.0f);
        return (water ? 25.0f : 14.0f) + 0.990f + h * 0.0085f;
    }

    static float LiquidSurfaceMaterial(bool water, float rawSeed) {
        return (water ? 25.0f : 14.0f) + rawSeed;
    }

    static float WaterLikeSurfaceMaterial(float seed, float rawSeed) {
        return 25.0f + rawSeed + std::fmod(std::abs(seed) * 0.0017f, 0.0009f);
    }

    static float WaterWallCanvasMaterial(float seed) {
        return 25.965f + std::fmod(std::abs(seed), 1.0f) * 0.0245f;
    }
