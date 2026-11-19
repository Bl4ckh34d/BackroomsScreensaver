    void BuildSavePointTypewriter(std::vector<Vertex>& model) {
        // Fractional slots select surfaces in the opaque typewriter shader.
        constexpr float enamel = 36.0000f;
        constexpr float frame = 36.0625f;
        constexpr float chrome = 36.1250f;
        constexpr float rubber = 36.1875f;
        constexpr float ivory = 36.2500f;
        constexpr float paper = 36.3125f;
        constexpr float wood = 36.3750f;
        constexpr float brass = 36.4375f;
        constexpr float ribbon = 36.5000f;
        const XMFLOAT3 xAxis{1, 0, 0};
        const XMFLOAT3 yAxis{0, 1, 0};
        const XMFLOAT3 zAxis{0, 0, 1};
        auto box = [&](XMFLOAT3 center, XMFLOAT3 half, float material) {
            AppendDynamicBoxAxes(model, center, xAxis, yAxis, zAxis, half, material);
        };
        auto triangle = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 normal,
                            XMFLOAT3 tangent, float material) {
            model.push_back({a, normal, tangent, {0, 0}, material});
            model.push_back({b, normal, tangent, {1, 0}, material});
            model.push_back({c, normal, tangent, {0.5f, 1}, material});
        };
        auto panel = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d, float material) {
            const XMFLOAT3 edge = Sub3(b, a);
            const XMFLOAT3 normal = Normalize3(Cross3(edge, Sub3(d, a)), yAxis);
            AppendDynamicQuad(model, a, b, c, d, normal, Normalize3(edge, xAxis), material);
        };
        auto cylinder = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 radial,
                            float radius, int sides, float material) {
            const XMFLOAT3 axis = Normalize3(Sub3(b, a), xAxis);
            const XMFLOAT3 perpendicular = Cross3(axis, radial);
            for (int side = 0; side < sides; ++side) {
                const float angle = (2.0f * kPi / sides) * side;
                const float next = angle + 2.0f * kPi / sides;
                const XMFLOAT3 n0 = Add3(Scale3(radial, std::cos(angle)), Scale3(perpendicular, std::sin(angle)));
                const XMFLOAT3 n1 = Add3(Scale3(radial, std::cos(next)), Scale3(perpendicular, std::sin(next)));
                const XMFLOAT3 a0 = Add3(a, Scale3(n0, radius));
                const XMFLOAT3 a1 = Add3(a, Scale3(n1, radius));
                const XMFLOAT3 b0 = Add3(b, Scale3(n0, radius));
                const XMFLOAT3 b1 = Add3(b, Scale3(n1, radius));
                model.insert(model.end(), {
                    {a0, n0, axis, {0, 0}, material}, {a1, n1, axis, {0, 1}, material},
                    {b1, n1, axis, {1, 1}, material}, {a0, n0, axis, {0, 0}, material},
                    {b1, n1, axis, {1, 1}, material}, {b0, n0, axis, {1, 0}, material}
                });
                triangle(a, a1, a0, Scale3(axis, -1.0f), radial, material);
                triangle(b, b0, b1, axis, radial, material);
            }
        };

        // Painted steel stand with a solid worktop.
        box({0, 0.675f, 0}, {0.338f, 0.024f, 0.247f}, wood);
        for (float x : {-0.278f, 0.278f}) {
            box({x, 0.620f, 0}, {0.017f, 0.032f, 0.207f}, frame);
            box({x, 0.165f, -0.016f}, {0.012f, 0.012f, 0.180f}, frame);
            for (float z : {-0.190f, 0.190f}) {
                box({x, 0.332f, z}, {0.016f, 0.316f, 0.016f}, frame);
                box({x, 0.010f, z}, {0.020f, 0.010f, 0.020f}, rubber);
            }
        }
        box({0, 0.619f, -0.191f}, {0.288f, 0.022f, 0.013f}, frame);
        box({0, 0.619f, 0.191f}, {0.288f, 0.022f, 0.013f}, frame);
        box({0, 0.168f, -0.191f}, {0.278f, 0.013f, 0.012f}, frame);
        for (float x : {-0.182f, 0.182f}) {
            for (float z : {-0.128f, 0.128f}) {
                box({x, 0.703f, z}, {0.018f, 0.005f, 0.016f}, rubber);
            }
        }

        auto deckY = [](float z) { return 0.815f - z * 0.20f; };
        auto rim = [](float w, float d, float bevel, auto height) {
            return std::array<XMFLOAT3, 8>{{
                {-w + bevel, height(d), d}, {w - bevel, height(d), d},
                {w, height(d - bevel), d - bevel}, {w, height(-d + bevel), -d + bevel},
                {w - bevel, height(-d), -d}, {-w + bevel, height(-d), -d},
                {-w, height(-d + bevel), -d + bevel}, {-w, height(d - bevel), d - bevel}
            }};
        };
        const auto bottom = rim(0.238f, 0.174f, 0.024f, [](float) { return 0.703f; });
        const auto seam = rim(0.254f, 0.187f, 0.026f, [](float) { return 0.732f; });
        const auto shoulder = rim(0.254f, 0.187f, 0.028f, [&](float z) { return deckY(z) - 0.019f; });
        const auto top = rim(0.239f, 0.174f, 0.027f, deckY);
        for (size_t i = 0; i < top.size(); ++i) {
            const size_t j = (i + 1) % top.size();
            panel(bottom[i], bottom[j], seam[j], seam[i], frame);
            panel(seam[i], seam[j], shoulder[j], shoulder[i], enamel);
            panel(shoulder[i], shoulder[j], top[j], top[i], enamel);
        }
        const XMFLOAT3 deckNormal = Normalize3({0.0f, 1.0f, 0.20f}, yAxis);
        for (size_t i = 1; i + 1 < top.size(); ++i) {
            triangle(top[0], top[i], top[i + 1], deckNormal, xAxis, enamel);
        }
        auto deck = [&](float x, float z, float lift) { return XMFLOAT3{x, deckY(z) + lift, z}; };
        panel(deck(-0.205f, 0.137f, 0.002f), deck(0.205f, 0.137f, 0.002f),
              deck(0.205f, -0.066f, 0.002f), deck(-0.205f, -0.066f, 0.002f), frame);
        box({0, 0.755f, 0.188f}, {0.060f, 0.010f, 0.002f}, brass);
        box({0, 0.755f, 0.191f}, {0.042f, 0.0014f, 0.0011f}, frame);

        constexpr uint16_t letters[] = {
            0b010'101'111'101'101, 0b110'101'110'101'110, 0b011'100'100'100'011, 0b110'101'101'101'110,
            0b111'100'110'100'111, 0b111'100'110'100'100, 0b011'100'101'101'011, 0b101'101'111'101'101,
            0b111'010'010'010'111, 0b001'001'001'101'010, 0b101'101'110'101'101, 0b100'100'100'100'111,
            0b101'111'111'101'101, 0b101'111'111'111'101, 0b010'101'101'101'010, 0b110'101'110'100'100,
            0b010'101'101'111'011, 0b110'101'110'101'101, 0b011'100'010'001'110, 0b111'010'010'010'010,
            0b101'101'101'101'111, 0b101'101'101'101'010, 0b101'101'111'111'101, 0b101'101'010'101'101,
            0b101'101'010'010'010, 0b111'001'010'100'111
        };
        constexpr uint16_t digits[] = {
            0b111'101'101'101'111, 0b010'110'010'010'111, 0b110'001'010'100'111, 0b110'001'010'001'110,
            0b101'101'111'001'001, 0b111'100'110'001'110, 0b011'100'111'101'111, 0b111'001'010'010'010,
            0b111'101'111'101'111, 0b111'101'111'001'110
        };
        const std::array<std::string, 4> rows{"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM,."};
        for (size_t row = 0; row < rows.size(); ++row) {
            const float z = -0.041f + static_cast<float>(row) * 0.043f;
            const std::string& keys = rows[row];
            for (size_t col = 0; col < keys.size(); ++col) {
                const float x = ((keys.size() - 1) * 0.5f - static_cast<float>(col)) * 0.037f;
                const XMFLOAT3 seat = deck(x, z, 0.005f);
                const XMFLOAT3 collar = Add3(seat, {0, 0.010f, 0});
                const XMFLOAT3 cap = Add3(collar, {0, 0.004f, 0});
                cylinder(seat, collar, xAxis, 0.0152f, 8, chrome);
                cylinder(collar, cap, xAxis, 0.0138f, 8, rubber);
                const char key = keys[col];
                const uint16_t glyph = key >= 'A' && key <= 'Z' ? letters[key - 'A'] :
                    key >= '0' && key <= '9' ? digits[key - '0'] : key == '.' ? 0b010 : 0b010'100;
                for (int bit = 0; bit < 15; ++bit) {
                    if ((glyph & (1u << (14 - bit))) == 0) continue;
                    const float l = x + (0.5f - bit % 3) * 0.0025f;
                    const float b = z + (bit / 3 - 2.5f) * 0.0025f;
                    const float y = cap.y + 0.0008f;
                    panel({l, y, b + 0.0025f}, {l + 0.0025f, y, b + 0.0025f},
                          {l + 0.0025f, y, b}, {l, y, b}, ivory);
                }
            }
        }
        box(deck(0, 0.150f, 0.009f), {0.093f, 0.008f, 0.008f}, rubber);

        // Carriage, ribbon guide, and coated steel feed roller.
        box({0, 0.846f, -0.115f}, {0.242f, 0.019f, 0.052f}, frame);
        for (float z : {-0.155f, -0.077f}) {
            cylinder({-0.254f, 0.859f, z}, {0.254f, 0.859f, z}, yAxis, 0.005f, 8, chrome);
        }
        cylinder({-0.247f, 0.874f, -0.115f}, {0.247f, 0.874f, -0.115f}, yAxis, 0.024f, 16, rubber);
        for (float sign : {-1.0f, 1.0f}) {
            cylinder({sign * 0.237f, 0.874f, -0.115f}, {sign * 0.260f, 0.874f, -0.115f}, yAxis, 0.022f, 12, chrome);
            cylinder({sign * 0.260f, 0.874f, -0.115f}, {sign * 0.283f, 0.874f, -0.115f}, yAxis, 0.020f, 12, frame);
            box({sign * 0.070f, 0.858f, -0.063f}, {0.030f, 0.009f, 0.014f}, ribbon);
        }
        AppendSegmentBox(model, {0.209f, 0.879f, -0.150f}, {0.294f, 0.911f, -0.104f},
            0.004f, 0.005f, chrome);
        AppendSegmentBox(model, {0.294f, 0.911f, -0.104f}, {0.297f, 0.916f, -0.066f},
            0.007f, 0.004f, frame);

        constexpr std::array<XMFLOAT2, 6> feed{{
            {0.851f, -0.129f}, {0.880f, -0.142f}, {0.918f, -0.147f},
            {0.961f, -0.160f}, {1.004f, -0.183f}, {1.041f, -0.217f}
        }};
        auto sheetEdge = [&](size_t i, float side) {
            return XMFLOAT3{side * 0.107f, feed[i].x + (side > 0 ? i * 0.0009f : 0.0f), feed[i].y};
        };
        for (size_t i = 0; i + 1 < feed.size(); ++i) {
            const float scale = 1.0f / (feed.size() - 1);
            const XMFLOAT3 a = sheetEdge(i, -1), b = sheetEdge(i, 1);
            const XMFLOAT3 c = sheetEdge(i + 1, 1), d = sheetEdge(i + 1, -1);
            XMFLOAT3 normal = Normalize3(Cross3(Sub3(b, a), Sub3(d, a)), zAxis);
            AppendDynamicQuadUV(model, a, b, c, d, normal, xAxis,
                {0, 1 - i * scale}, {1, 1 - i * scale}, {1, 1 - (i + 1) * scale}, {0, 1 - (i + 1) * scale}, paper);
        }
        cylinder({-0.143f, 0.901f, -0.128f}, {0.143f, 0.901f, -0.128f}, yAxis, 0.004f, 8, chrome);
    }
