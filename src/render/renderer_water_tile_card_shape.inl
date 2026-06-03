        float vMode = static_cast<float>(surface.mode + neighborMask * 8);
        float material = WaterDecalMaterial(surface.seed, 0.0f, 0.014f);
        float h0 = LampHash(surface.seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y) + 3.1f);
        float h1 = LampHash(surface.seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x) + 5.7f);
        float h2 = LampHash(surface.seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);
        float h3 = LampHash(surface.seed * 41.0f + static_cast<float>(t.x) * 1.7f, static_cast<float>(t.y) * 2.3f);
        float sizeScore = std::clamp(0.70f + (surface.score - 0.75f) * 0.22f, 0.54f, 1.05f);
        float halfW = tileW * (ceiling ? (0.30f + h0 * 0.20f) : (0.20f + h0 * 0.17f)) * sizeScore;
        float halfD = tileD * (ceiling ? (0.30f + h1 * 0.20f) : (0.18f + h1 * 0.16f)) * sizeScore;
        if (surface.mode == 3) {
            halfW *= 0.62f;
            halfD *= 0.62f;
        }
        float cx = (l + r) * 0.5f + (h2 - 0.5f) * tileW * (ceiling ? 0.18f : 0.24f);
        float cz = (z0 + z1) * 0.5f + (h3 - 0.5f) * tileD * (ceiling ? 0.18f : 0.22f);
        auto pullTowardSide = [&](int side, float amount) {
            if (side == 0) cz -= tileD * amount;
            else if (side == 1) cz += tileD * amount;
            else if (side == 2) cx -= tileW * amount;
            else cx += tileW * amount;
        };
        if (surface.mode > 0 && surface.mode != 3) {
            pullTowardSide(surface.side, ceiling ? 0.16f : 0.13f);
            if (surface.side == 0 || surface.side == 1) halfD = std::max(halfD, tileD * (ceiling ? 0.43f : 0.36f));
            else halfW = std::max(halfW, tileW * (ceiling ? 0.43f : 0.36f));
        }
