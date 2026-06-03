    void AppendDynamicDoor(std::vector<Vertex>& verts) {
        float halfW = 0.60f;
        float halfH = 1.05f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        float angle = sessionRuntime_.mode == RendererRuntimeMode::MainMenu
            ? std::min(exitDoorPresentation_.angle * 1.12f, 1.54f)
            : exitDoorPresentation_.angle;
        XMFLOAT3 right = RotateYVec(exitDoorPresentation_.right, angle);
        XMFLOAT3 normal = RotateYVec(exitDoorPresentation_.normal, angle);
        XMFLOAT3 center = Add3(exitDoorPresentation_.hinge, Add3(Scale3(right, halfW), Scale3(normal, 0.012f)));
        AppendDynamicBoxAxes(verts, center, right, up, normal, {halfW, halfH, 0.030f}, 6.0f);

        auto localCenter = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, normal, x, y, z));
        };

        auto handle = [&](float zSign) {
            XMFLOAT3 plateCenter = localCenter(halfW * 0.63f, -0.08f, zSign * 0.044f);
            AppendDynamicBoxAxes(verts, plateCenter, right, up, normal, {0.050f, 0.115f, 0.005f}, 10.0f);
            XMFLOAT3 neckCenter = localCenter(halfW * 0.63f, -0.08f, zSign * 0.062f);
            AppendDynamicBoxAxes(verts, neckCenter, right, up, normal, {0.030f, 0.026f, 0.018f}, 10.0f);
            XMFLOAT3 leverCenter = localCenter(halfW * 0.63f - 0.090f, -0.08f, zSign * 0.076f);
            AppendDynamicBoxAxes(verts, leverCenter, right, up, normal, {0.125f, 0.015f, 0.016f}, 10.0f);
            XMFLOAT3 leverTip = localCenter(halfW * 0.63f - 0.216f, -0.08f, zSign * 0.076f);
            AppendDynamicBoxAxes(verts, leverTip, right, up, normal, {0.010f, 0.020f, 0.018f}, 10.0f);
        };
        handle(1.0f);
        handle(-1.0f);

    }
