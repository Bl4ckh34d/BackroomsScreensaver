        float smoothDt = std::clamp(timeRuntime_.time - monsterPresentation_.bodySmoothTime, 0.0f, 0.05f);
        bool resetBodySmoothing = monsterPresentation_.smoothedBodyPoints.size() != static_cast<size_t>(bodyCount) ||
            monsterPresentation_.smoothedBodyUps.size() != static_cast<size_t>(bodyCount) ||
            monsterPresentation_.bodySmoothTime < -100.0f ||
            smoothDt <= 0.0001f;
        if (resetBodySmoothing) {
            monsterPresentation_.smoothedBodyPoints.assign(bodyPoints.begin(), bodyPoints.begin() + bodyCount);
            monsterPresentation_.smoothedBodyUps.assign(bodyUps.begin(), bodyUps.begin() + bodyCount);
        } else {
            for (int i = 0; i < bodyCount; ++i) {
                float follow = Lerp(14.0f, 6.0f, static_cast<float>(i) / std::max(1.0f, static_cast<float>(bodyCount - 1)));
                float alpha = 1.0f - std::exp(-smoothDt * follow);
                XMFLOAT3 target = bodyPoints[static_cast<size_t>(i)];
                XMFLOAT3 prev = monsterPresentation_.smoothedBodyPoints[static_cast<size_t>(i)];
                if (Length3(Sub3(target, prev)) > maze.TileAverage() * 1.6f) {
                    monsterPresentation_.smoothedBodyPoints[static_cast<size_t>(i)] = target;
                } else {
                    monsterPresentation_.smoothedBodyPoints[static_cast<size_t>(i)] = Lerp3(prev, target, alpha);
                }
                XMFLOAT3 prevUp = monsterPresentation_.smoothedBodyUps[static_cast<size_t>(i)];
                XMFLOAT3 targetUp = bodyUps[static_cast<size_t>(i)];
                if (Dot3(prevUp, targetUp) < 0.0f) targetUp = Scale3(targetUp, -1.0f);
                monsterPresentation_.smoothedBodyUps[static_cast<size_t>(i)] = Normalize3(Lerp3(prevUp, targetUp, 1.0f - std::exp(-smoothDt * 4.8f)), targetUp);
            }
            std::copy(monsterPresentation_.smoothedBodyPoints.begin(), monsterPresentation_.smoothedBodyPoints.begin() + bodyCount, bodyPoints.begin());
            std::copy(monsterPresentation_.smoothedBodyUps.begin(), monsterPresentation_.smoothedBodyUps.begin() + bodyCount, bodyUps.begin());
            constrainBodyChain();
            std::copy(bodyPoints.begin(), bodyPoints.begin() + bodyCount, monsterPresentation_.smoothedBodyPoints.begin());
        }
        monsterPresentation_.bodySmoothTime = timeRuntime_.time;

        for (int i = 0; i < bodyCount; ++i) {
            XMFLOAT3 tangent = bodyChainTangent(i, bodyTangents[static_cast<size_t>(i)]);
            bodyTangents[static_cast<size_t>(i)] = tangent;
            XMFLOAT3 upProjected = Normalize3(Sub3(bodyUps[static_cast<size_t>(i)], Scale3(tangent, Dot3(bodyUps[static_cast<size_t>(i)], tangent))),
                i > 0 ? bodyUps[static_cast<size_t>(i - 1)] : up);
            if (i > 0 && Dot3(upProjected, bodyUps[static_cast<size_t>(i - 1)]) < 0.0f) {
                upProjected = Scale3(upProjected, -1.0f);
            }
            bodyUps[static_cast<size_t>(i)] = upProjected;
            bodySides[static_cast<size_t>(i)] = Normalize3(Cross3(bodyUps[static_cast<size_t>(i)], tangent), bodySides[static_cast<size_t>(i)]);
        }
