    void AddOrientedBox(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                        XMFLOAT3 center, XMFLOAT3 half, float yaw, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        auto p = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, forward, x * half.x, y * half.y, z * half.z));
        };
        auto face = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c0, XMFLOAT3 d, XMFLOAT3 n, XMFLOAT3 t) {
            AddQuadUV(v, i, a, b, c0, d, n, t, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
        };
        face(p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right);
        face(p( 1, -1, -1), p(-1, -1, -1), p(-1,  1, -1), p( 1,  1, -1), Scale3(forward, -1.0f), Scale3(right, -1.0f));
        face(p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f));
        face(p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward);
        face(p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right);
        face(p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right);
    }
