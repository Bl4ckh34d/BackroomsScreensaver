        auto clampBodyPoint = [&](int idx) {
            float radius = bodyRadii[static_cast<size_t>(idx)];
            bodyPoints[static_cast<size_t>(idx)].y = std::clamp(bodyPoints[static_cast<size_t>(idx)].y,
                radius * 0.82f + 0.045f, settingsRuntime_.live.wallHeightMeters - radius * 0.64f - 0.026f);
        };
        auto constrainBodyChain = [&]() {
            for (int iter = 0; iter < 3; ++iter) {
                for (int i = 1; i < bodyCount; ++i) {
                    XMFLOAT3& a = bodyPoints[static_cast<size_t>(i - 1)];
                    XMFLOAT3& b = bodyPoints[static_cast<size_t>(i)];
                    XMFLOAT3 delta = Sub3(b, a);
                    float len = Length3(delta);
                    if (len <= 0.001f) continue;
                    XMFLOAT3 dir = Scale3(delta, 1.0f / len);
                    float targetLen = visualBodySpacing * Lerp(0.88f, 1.18f,
                        static_cast<float>(i) / std::max(1.0f, static_cast<float>(bodyCount - 1)));
                    float minLen = std::max(targetLen * 0.62f,
                        (bodyRadii[static_cast<size_t>(i - 1)] + bodyRadii[static_cast<size_t>(i)]) * 0.34f);
                    if (len < minLen) {
                        float push = (minLen - len) * 0.46f;
                        if (i > 1) a = Add3(a, Scale3(dir, -push * 0.45f));
                        b = Add3(b, Scale3(dir, push));
                    } else if (len > targetLen * 1.42f) {
                        float pull = (len - targetLen * 1.42f) * 0.28f;
                        b = Add3(b, Scale3(dir, -pull));
                    }
                    clampBodyPoint(i - 1);
                    clampBodyPoint(i);
                }
                for (int i = 2; i < bodyCount; ++i) {
                    XMFLOAT3 prev = Sub3(bodyPoints[static_cast<size_t>(i - 1)], bodyPoints[static_cast<size_t>(i - 2)]);
                    XMFLOAT3 cur = Sub3(bodyPoints[static_cast<size_t>(i)], bodyPoints[static_cast<size_t>(i - 1)]);
                    float prevLen = Length3(prev);
                    float curLen = Length3(cur);
                    if (prevLen <= 0.001f || curLen <= 0.001f) continue;
                    XMFLOAT3 prevDir = Scale3(prev, 1.0f / prevLen);
                    XMFLOAT3 curDir = Scale3(cur, 1.0f / curLen);
                    constexpr float kMinSegmentDot = 0.42f;
                    float bendDot = Dot3(prevDir, curDir);
                    if (bendDot < kMinSegmentDot) {
                        float correction = Clamp01((kMinSegmentDot - bendDot) / (1.0f + kMinSegmentDot));
                        XMFLOAT3 softenedDir = Normalize3(Lerp3(curDir, prevDir, correction * 0.72f), prevDir);
                        bodyPoints[static_cast<size_t>(i)] = Add3(bodyPoints[static_cast<size_t>(i - 1)], Scale3(softenedDir, curLen));
                        clampBodyPoint(i);
                    }
                }
                for (int i = 0; i < bodyCount; ++i) {
                    for (int j = i + 3; j < bodyCount; ++j) {
                        XMFLOAT3 delta = Sub3(bodyPoints[static_cast<size_t>(j)], bodyPoints[static_cast<size_t>(i)]);
                        float len = Length3(delta);
                        float minLen = (bodyRadii[static_cast<size_t>(i)] + bodyRadii[static_cast<size_t>(j)]) * 0.82f;
                        if (len <= 0.001f || len >= minLen) continue;
                        XMFLOAT3 dir = Scale3(delta, 1.0f / len);
                        float push = (minLen - len) * 0.52f;
                        if (i > 0) bodyPoints[static_cast<size_t>(i)] = Add3(bodyPoints[static_cast<size_t>(i)], Scale3(dir, -push * 0.45f));
                        bodyPoints[static_cast<size_t>(j)] = Add3(bodyPoints[static_cast<size_t>(j)], Scale3(dir, push));
                        clampBodyPoint(i);
                        clampBodyPoint(j);
                    }
                }
            }
        };
        constrainBodyChain();
