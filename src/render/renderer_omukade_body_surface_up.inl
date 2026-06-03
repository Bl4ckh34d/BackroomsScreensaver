            XMFLOAT3 surfaceUp = Normalize3(best.normal, up);
            if (allowBodySurfaceClimb && i == 0 && hasHandSupportUp) {
                surfaceUp = handSupportUp;
                best.center = Add3(bodyCenterlinePoints[0], Scale3(surfaceUp, radius * 0.98f + 0.026f));
                best.center.y = std::clamp(best.center.y, radius * 0.74f + 0.026f,
                    settingsRuntime_.live.wallHeightMeters - radius * 0.74f - 0.026f);
            }
            if (i > 0) {
                XMFLOAT3 prevRawUp = Normalize3(bodyUps[static_cast<size_t>(i - 1)], up);
                bool directFloorCeilingFlip = std::abs(prevRawUp.y) > 0.70f &&
                    std::abs(surfaceUp.y) > 0.70f &&
                    Dot3(prevRawUp, surfaceUp) < -0.35f;
                if (directFloorCeilingFlip) {
                    surfaceUp = prevRawUp;
                    best.normal = prevRawUp;
                    best.center = centerForSurface(prevRawUp);
                }
            }
            if (i == 0) {
                blobSurfaceUp = surfaceUp;
            } else {
                surfaceUp = blobSurfaceUp;
                best.center = centerForSurface(surfaceUp);
            }
            if (i > 0) {
                XMFLOAT3 prevUp = bodyUps[static_cast<size_t>(i - 1)];
                float continuity = 0.72f;
                surfaceUp = Normalize3(Lerp3(surfaceUp, prevUp, continuity), prevUp);
                if (Dot3(surfaceUp, prevUp) < 0.0f) surfaceUp = Scale3(surfaceUp, -1.0f);
                surfaceUp = Normalize3(Sub3(surfaceUp, Scale3(tangent, Dot3(surfaceUp, tangent))), prevUp);
            }
