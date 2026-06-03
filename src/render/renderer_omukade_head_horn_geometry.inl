        if (false && !externalSkull) {
            auto hornPoint = [&](float x, float y, float z) {
                return Add3(skull, OrientedOffset(hRight, hUp, hForward, x * modelXZ, y * modelY, z * modelXZ));
            };
            for (int side = -1; side <= 1; side += 2) {
                XMFLOAT3 h0 = hornPoint(0.10f * side, 0.11f, -0.035f);
                XMFLOAT3 h1 = hornPoint(0.30f * side, 0.38f, -0.080f);
                XMFLOAT3 h2 = hornPoint(0.56f * side, 0.55f, -0.120f);
                XMFLOAT3 h3 = hornPoint(0.75f * side, 0.61f, -0.080f);
                seg(h0, h1, 0.028f, 0.023f, boneMat);
                seg(h1, h2, 0.024f, 0.020f, boneMat);
                seg(h2, h3, 0.018f, 0.016f, boneMat);
                seg(h1, hornPoint(0.24f * side, 0.58f, 0.040f), 0.016f, 0.014f, boneMat);
                seg(h2, hornPoint(0.54f * side, 0.76f, -0.020f), 0.014f, 0.012f, boneMat);
                seg(h2, hornPoint(0.68f * side, 0.46f, 0.060f), 0.014f, 0.012f, boneMat);
            }
        }
