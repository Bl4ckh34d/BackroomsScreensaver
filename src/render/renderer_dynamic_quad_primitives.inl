    void AppendDynamicQuad(std::vector<Vertex>& verts,
                           XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                           XMFLOAT3 normal, XMFLOAT3 tangent, float material) {
        XMFLOAT2 uvs[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
        XMFLOAT3 pos[4] = {a, b, c, d};
        int order[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; ++i) {
            int idx = order[i];
            verts.push_back({pos[idx], normal, tangent, uvs[idx], material});
        }
    }

    void AppendDynamicQuadUV(std::vector<Vertex>& verts,
                             XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                             XMFLOAT3 normal, XMFLOAT3 tangent,
                             XMFLOAT2 auv, XMFLOAT2 buv, XMFLOAT2 cuv, XMFLOAT2 duv,
                             float material) {
        XMFLOAT2 uvs[4] = {auv, buv, cuv, duv};
        XMFLOAT3 pos[4] = {a, b, c, d};
        int order[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; ++i) {
            int idx = order[i];
            verts.push_back({pos[idx], normal, tangent, uvs[idx], material});
        }
    }
