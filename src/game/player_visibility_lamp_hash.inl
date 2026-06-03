// Player camera attention visibility prop.

    static float Frac(float v) {
        return v - std::floor(v);
    }

    static float LampHash(float x, float z) {
        float px = Frac(x * 123.34f);
        float pz = Frac(z * 456.21f);
        float d = px * (px + 45.32f) + pz * (pz + 45.32f);
        px += d;
        pz += d;
        return Frac(px * pz);
    }

    float LampSeed(int cellX, int cellZ) const {
        return LampHash(static_cast<float>(cellX), static_cast<float>(cellZ));
    }

    bool LampBrokenZone(int cellX, int cellZ) const {
        return LampHash(static_cast<float>(cellX) + 83.7f, static_cast<float>(cellZ) - 29.4f) >= 1.0f - settingsRuntime_.live.brokenZoneRatio;
    }

    bool LampIsOn(int cellX, int cellZ) const {
        return !LampBrokenZone(cellX, cellZ) && LampSeed(cellX, cellZ) >= 1.0f - settingsRuntime_.live.lampOnRatio;
    }
