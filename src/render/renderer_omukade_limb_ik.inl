                XMFLOAT3 toTarget = Sub3(target, root);
                float reach = Length3(toTarget);
                XMFLOAT3 reachDir = Normalize3(toTarget, sideDir);
                float clampedReach = std::clamp(reach, 0.05f, upperLen + lowerLen - 0.025f);
                float along = (upperLen * upperLen - lowerLen * lowerLen + clampedReach * clampedReach) / std::max(0.001f, 2.0f * clampedReach);
                float bendHeight = std::sqrt(std::max(0.0f, upperLen * upperLen - along * along));
                XMFLOAT3 bendAxis = Normalize3(Add3(Scale3(up, 0.82f), Add3(Scale3(sideDir, 0.34f), Scale3(tangent, std::sin(phase) * 0.30f))), up);
                if (target.y < 0.08f) {
                    bendAxis = Normalize3(Add3(Scale3(sideDir, 0.72f), Add3(Scale3(up, 0.42f), Scale3(tangent, std::sin(phase) * 0.22f))), sideDir);
                } else if (target.y > settingsRuntime_.live.wallHeightMeters - 0.12f) {
                    bendAxis = Normalize3(Add3(Scale3(sideDir, 0.65f), Add3(Scale3(up, -0.48f), Scale3(tangent, std::sin(phase) * 0.22f))), sideDir);
                }
                XMFLOAT3 knee = Add3(root, Add3(Scale3(reachDir, along), Scale3(bendAxis, bendHeight * (0.58f + swing * 0.24f))));
                knee = Add3(knee, Add3(Scale3(tangent, std::sin(phase * 0.67f) * 0.045f), Scale3(localUp, swing * 0.045f)));
