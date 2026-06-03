        int tiles = std::clamp(gDebugSliceTiles, 1, 5);
        float centerX = ctx.ox + (1.0f + static_cast<float>(tiles) * 0.5f) * ctx.tileW;
        float centerZ = ctx.oz + (1.0f + static_cast<float>(tiles) * 0.5f) * ctx.tileD;
        float targetMax = 1.22f;
        float yaw = kPi;
        float pitch = 0.0f;
        switch (propIndex) {
        case 3:
            pitch = kPi * 0.5f;
            targetMax = 1.12f;
            break;
        case 6:
            targetMax = 0.58f;
            break;
        case 7:
            pitch = kPi * 0.5f;
            targetMax = 0.62f;
            break;
        case 4:
            targetMax = 1.44f;
            break;
        case 5:
            targetMax = 1.62f;
            yaw = kPi * 0.5f;
            break;
        case 8:
            targetMax = 0.56f;
            break;
        case 9:
            targetMax = 0.58f;
            break;
        case 10:
            targetMax = 0.86f;
            break;
        case 11:
            targetMax = 1.18f;
            break;
        case 12:
        case 13:
        case 14:
        case 15:
            targetMax = 1.36f;
            yaw = kPi * 0.5f;
            break;
        default:
            break;
        }
