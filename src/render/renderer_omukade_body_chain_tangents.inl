        auto bodyChainTangent = [&](int idx, XMFLOAT3 fallback) {
            if (bodyCount <= 1) return Normalize3(fallback, monsterForward);
            XMFLOAT3 tangent{};
            if (idx <= 0) {
                tangent = Sub3(bodyPoints[0], bodyPoints[1]);
            } else if (idx + 1 >= bodyCount) {
                tangent = Sub3(bodyPoints[static_cast<size_t>(idx - 1)], bodyPoints[static_cast<size_t>(idx)]);
            } else {
                tangent = Sub3(bodyPoints[static_cast<size_t>(idx - 1)], bodyPoints[static_cast<size_t>(idx + 1)]);
            }
            return Normalize3(tangent, fallback);
        };
        for (int i = 0; i < bodyCount; ++i) {
            XMFLOAT3 tangent = bodyChainTangent(i, i > 0 ? bodyTangents[static_cast<size_t>(i - 1)] : monsterForward);
            XMFLOAT3 side = Normalize3(Cross3(up, tangent), monsterRight);
            bodySides[static_cast<size_t>(i)] = side;
            bodyTangents[static_cast<size_t>(i)] = tangent;
        }
