        auto slerpDirection = [&](XMFLOAT3 from, XMFLOAT3 to, float t) {
            from = Normalize3(from, to);
            to = Normalize3(to, from);
            t = Clamp01(t);
            float dot = std::clamp(Dot3(from, to), -0.9995f, 0.9995f);
            if (dot > 0.9990f) return Normalize3(Lerp3(from, to, t), to);
            float theta = std::acos(dot) * t;
            XMFLOAT3 rel = Normalize3(Sub3(to, Scale3(from, dot)), to);
            return Normalize3(Add3(Scale3(from, std::cos(theta)), Scale3(rel, std::sin(theta))), to);
        };
        auto keepHeadOnSurface = [&](bool projectForward = true) {
            if (projectForward) {
                hForward = Normalize3(Sub3(hForward, Scale3(hUp, Dot3(hForward, hUp))), monsterForward);
            } else {
                hForward = Normalize3(hForward, monsterForward);
            }
            hRight = Normalize3(Cross3(hUp, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        };
