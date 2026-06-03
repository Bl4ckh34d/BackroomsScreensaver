    void AppendMenuDoorwayLight(std::vector<Vertex>& transparentVerts) {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return;
        float openT = SmoothStep(0.24f, 1.30f, exitDoorPresentation_.angle);
        openT *= openT;
        if (openT <= 0.001f) return;

        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 right = Normalize3(exitDoorPresentation_.right, {1.0f, 0.0f, 0.0f});
        XMFLOAT3 inward = Normalize3(exitDoorPresentation_.normal, {0.0f, 0.0f, 1.0f});
        float openMat = 19.54f + openT * 0.12f;

        XMFLOAT3 aperture = Add3(exitDoorPresentation_.center, Add3(Scale3(inward, 0.66f), {0.0f, -0.46f, 0.0f}));
        XMFLOAT3 floorHit = Add3(exitDoorPresentation_.center, Add3(Scale3(inward, 2.95f), {0.0f, -0.92f, 0.0f}));
        XMFLOAT3 farDust = Add3(exitDoorPresentation_.center, Add3(Scale3(inward, 3.90f), {0.0f, -0.72f, 0.0f}));
        XMFLOAT3 nearSide = Scale3(right, 0.38f);
        XMFLOAT3 floorSide = Scale3(right, 1.46f);
        XMFLOAT3 dustSide = Scale3(right, 0.86f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(aperture, Scale3(nearSide, -1.0f)),
            Add3(aperture, nearSide),
            Add3(floorHit, floorSide),
            Add3(floorHit, Scale3(floorSide, -1.0f)),
            up, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, openMat + 0.035f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(aperture, Add3(Scale3(dustSide, -0.72f), Scale3(up, -0.06f))),
            Add3(aperture, Add3(Scale3(dustSide, 0.72f), Scale3(up, -0.06f))),
            Add3(farDust, Add3(Scale3(dustSide, 1.0f), Scale3(up, 0.10f))),
            Add3(farDust, Add3(Scale3(dustSide, -1.0f), Scale3(up, 0.10f))),
            up, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, openMat + 0.050f);
    }
