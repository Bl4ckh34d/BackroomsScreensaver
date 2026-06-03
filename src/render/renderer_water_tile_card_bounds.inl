        if (!surface.active || surface.suppressCard) return;

        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        float l = build.surface.ox + static_cast<float>(t.x) * tileW;
        float r = l + tileW;
        float z0 = build.surface.oz + static_cast<float>(t.y) * tileD;
        float z1 = z0 + tileD;
        float y = ceiling ? build.ceilingY : build.floorLift;
        float u = static_cast<float>(surface.side);
        int neighborMask = 0;
