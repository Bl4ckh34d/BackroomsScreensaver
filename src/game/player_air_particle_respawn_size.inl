        float sizeRoll = RandRange(0.0f, 1.0f);
        float sizeT = RandRange(0.0f, 1.0f);
        float baseSize = 0.0f;
        if (sizeRoll < 0.50f) {
            baseSize = Lerp(0.0018f, 0.0048f, std::pow(sizeT, 1.35f));
        } else if (sizeRoll < 0.90f) {
            baseSize = Lerp(0.0048f, 0.0083f, sizeT);
        } else {
            baseSize = Lerp(0.0083f, 0.0114f, std::sqrt(sizeT));
        }
        float layerScale = p.nearLayer > 1.5f ? RandRange(1.28f, 2.02f) : (p.nearLayer > 0.5f ? RandRange(1.10f, 1.62f) : 1.0f);
        p.size = baseSize * layerScale * std::clamp(settingsRuntime_.live.airParticleSize, 0.20f, 4.0f);
        float aspectRoll = RandRange(0.0f, 1.0f);
        if (aspectRoll < 0.34f) {
            p.aspect = RandRange(1.95f, 4.10f);
        } else if (aspectRoll < 0.66f) {
            p.aspect = RandRange(0.24f, 0.56f);
        } else {
            p.aspect = RandRange(0.62f, 1.62f);
        }
