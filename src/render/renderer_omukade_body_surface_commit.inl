            p = best.center;
            float gait = timeRuntime_.time * 3.65f - fi * 0.74f + monsterPosition.x * 0.09f + monsterPosition.z * 0.05f;
            float stepLift = std::pow(Clamp01(std::sin(gait) * 0.5f + 0.5f), 2.3f);
            float gutPulse = std::sin(timeRuntime_.time * 2.65f - fi * 0.58f) * 0.065f;
            float bodyBounce = std::sin(timeRuntime_.time * 3.75f - fi * 0.92f) * 0.026f * (0.55f + taper * 0.45f);
            float curiosityLift = curiosityPose * std::pow(taper, 2.4f) * (0.46f * modelY);
            p = Add3(p, Scale3(surfaceUp, 0.045f * modelY + stepLift * 0.018f + bodyBounce * 0.30f + gutPulse * 0.18f + curiosityLift * 0.36f));
            bodyUps[static_cast<size_t>(i)] = surfaceUp;
            bodySides[static_cast<size_t>(i)] = Normalize3(Cross3(surfaceUp, tangent), side);
            bodyPoints[static_cast<size_t>(i)] = p;
        }
