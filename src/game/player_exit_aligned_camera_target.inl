    XMFLOAT3 ExitAlignedCameraTarget() const {
        XMFLOAT3 inward = Normalize3(exitDoorPresentation_.normal, {0.0f, 0.0f, 1.0f});
        float standOff = std::clamp(gameWorld_.maze.TileMinimum() * 0.46f, 0.78f, 1.08f);
        XMFLOAT3 target = Add3(exitDoorPresentation_.center, Scale3(inward, standOff));
        target.y = 1.43f;
        return target;
    }
