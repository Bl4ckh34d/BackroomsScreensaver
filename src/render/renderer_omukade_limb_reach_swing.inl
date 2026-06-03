            float swingT = Clamp01((now - anchor.swingStart) / std::max(0.001f, anchor.swingDuration));
            float travelT = SmoothStep(0.06f, 0.96f, swingT);
            float eased = travelT * travelT * travelT * (travelT * (travelT * 6.0f - 15.0f) + 10.0f);
            if (swingT >= 0.999f) {
                anchor.anchor = anchor.swingTo;
                anchor.anchorNormal = anchor.swingToNormal;
                anchor.planted = true;
                recordHandprint(anchor.anchor, anchor.anchorNormal, limbId, anchor.retargetCount);
                return LimbReachTarget{anchor.anchor, anchor.anchorNormal, 0.0f};
            }
            float liftT = std::sin(SmoothStep(0.0f, 1.0f, swingT) * kPi);
            float arc = liftT * (0.060f + Rand01(limbId, anchor.retargetCount + 853, sessionRuntime_.runtimeSeed) * 0.040f) * Lerp(1.0f, 1.18f, scramble);
            XMFLOAT3 arcDir = upDir;
            if (anchor.swingTo.y > settingsRuntime_.live.wallHeightMeters - 0.12f) {
                arcDir = Scale3(up, -1.0f);
            } else if (anchor.swingTo.y > 0.10f) {
                arcDir = Normalize3(Add3(upDir, Scale3(sideDir, 0.38f)), upDir);
            }
            float curl = std::sin(eased * kPi) * (0.008f + Rand01(limbId, anchor.retargetCount + 857, sessionRuntime_.runtimeSeed) * 0.010f) *
                Lerp(0.65f, 1.05f, scramble);
            XMFLOAT3 target = Add3(Lerp3(anchor.swingFrom, anchor.swingTo, eased),
                Add3(Scale3(arcDir, arc), Scale3(sideDir, curl)));
            XMFLOAT3 normal = Normalize3(Lerp3(anchor.swingFromNormal, anchor.swingToNormal, eased), up);
            return LimbReachTarget{target, normal, liftT};
        };
