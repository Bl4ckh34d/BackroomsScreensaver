        if (sourceFromCeiling) {
            float minTile = std::max(0.10f, std::min(tileW, tileD));
            float h0 = LampHash(seed * 67.0f + center.x, center.z);
            float h1 = LampHash(seed * 71.0f - center.z, center.x);
            float poolW = std::clamp(w * (0.78f + h0 * 0.42f), minTile * 0.30f, minTile * 0.82f);
            float poolD = minTile * (0.34f + h1 * 0.30f);
            float poolYaw = std::atan2(normal.x, normal.z);
            XMFLOAT3 poolCenter = Add3({center.x, 0.0f, center.z}, Scale3(normal, poolD * 0.48f + 0.020f));
            QueueWallWaterPoolCard(build, t, poolCenter.x, poolCenter.z, side, seed + 0.83f,
                poolW, poolD, poolYaw, 1.18f);
        }
