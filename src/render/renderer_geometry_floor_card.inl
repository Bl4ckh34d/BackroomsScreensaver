    void AddFloorCard(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                      XMFLOAT3 center, float width, float depth, float yaw, float y, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 d = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward, -depth * 0.5f)));
        a.y = b.y = c0.y = d.y = y;
        AddQuadUV(v, i, a, b, c0, d, {0, 1, 0}, right, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
    }
