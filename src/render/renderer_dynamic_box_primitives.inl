    void AppendDynamicBoxAxes(std::vector<Vertex>& verts, XMFLOAT3 center,
                              XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward,
                              XMFLOAT3 half, float material) {
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        auto p = [&](float x, float y, float z) {
            return Add3(center, Add3(Scale3(right, x * half.x), Add3(Scale3(up, y * half.y), Scale3(forward, z * half.z))));
        };
        AppendDynamicQuad(verts, p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right, material);
        AppendDynamicQuad(verts, p( 1, -1, -1), p(-1, -1, -1), p(-1,  1, -1), p( 1,  1, -1), Scale3(forward, -1.0f), Scale3(right, -1.0f), material);
        AppendDynamicQuad(verts, p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f), material);
        AppendDynamicQuad(verts, p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward, material);
        AppendDynamicQuad(verts, p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right, material);
        AppendDynamicQuad(verts, p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right, material);
    }

    void AppendDynamicBox(std::vector<Vertex>& verts, XMFLOAT3 center, XMFLOAT3 half, float yaw, float material) {
        XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
        AppendDynamicBoxAxes(verts, center, right, up, forward, half, material);
    }

    void AppendSegmentBox(std::vector<Vertex>& verts, XMFLOAT3 a, XMFLOAT3 b,
                          float halfWidth, float halfDepth, float material) {
        XMFLOAT3 axis = Sub3(b, a);
        float len = Length3(axis);
        if (len <= 0.001f) return;
        XMFLOAT3 upAxis = Scale3(axis, 1.0f / len);
        XMFLOAT3 ref{0.0f, 1.0f, 0.0f};
        if (std::abs(Dot3(upAxis, ref)) > 0.88f) ref = {1.0f, 0.0f, 0.0f};
        XMFLOAT3 right = Normalize3(Cross3(ref, upAxis), {1.0f, 0.0f, 0.0f});
        XMFLOAT3 forward = Normalize3(Cross3(upAxis, right), {0.0f, 0.0f, 1.0f});
        AppendDynamicBoxAxes(verts, Lerp3(a, b, 0.5f), right, upAxis, forward, {halfWidth, len * 0.5f, halfDepth}, material);
    }
