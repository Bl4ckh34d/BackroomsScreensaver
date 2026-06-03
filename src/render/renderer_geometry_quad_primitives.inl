// Renderer geometry primitive helpers.
// Included inside Renderer private section before mesh construction helpers.

    void AddQuad(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                 XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                 XMFLOAT3 n, XMFLOAT3 t, float material, float uScale, float vScale) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, {0.0f, 0.0f}, material});
        v.push_back({b, n, t, {uScale, 0.0f}, material});
        v.push_back({c, n, t, {uScale, vScale}, material});
        v.push_back({d, n, t, {0.0f, vScale}, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    void AddQuadUV(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                   XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                   XMFLOAT3 n, XMFLOAT3 t, XMFLOAT2 auv, XMFLOAT2 buv, XMFLOAT2 cuv, XMFLOAT2 duv,
                   float material) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, auv, material});
        v.push_back({b, n, t, buv, material});
        v.push_back({c, n, t, cuv, material});
        v.push_back({d, n, t, duv, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }
