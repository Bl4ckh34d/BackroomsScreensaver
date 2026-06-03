        int limbPairs = debugEffectMonster ? 8 : (monsterDetail >= 2 ? 10 : (monsterDetail == 1 ? 7 : 4));
        int requiredLimbAnchors = std::max(0, limbPairs * 2);
        if (monsterPresentation_.limbAnchors.size() < static_cast<size_t>(requiredLimbAnchors)) {
            monsterPresentation_.limbAnchors.resize(static_cast<size_t>(requiredLimbAnchors));
        }
        float gaitDrive = Clamp01(SmoothStep(0.0f, 1.0f, Clamp01(world.monsterRoamBurstTimer / 1.05f)) * (1.0f - monsterPresentation_.headChaseBlend * 0.65f) +
            SmoothStep(0.18f, 1.0f, monsterPresentation_.headChaseBlend) * 0.64f);
        float gaitStepInterval = Lerp(0.34f, 0.16f, gaitDrive);
        int gaitTick = requiredLimbAnchors > 0
            ? static_cast<int>(std::floor((timeRuntime_.time + static_cast<float>(sessionRuntime_.runtimeSeed & 31) * 0.013f) / std::max(0.035f, gaitStepInterval)))
            : 0;
        int gaitPair = limbPairs > 0 ? gaitTick % limbPairs : 0;
        int gaitWave = limbPairs > 0 ? gaitTick / limbPairs : 0;
        bool gaitRightSide = ((gaitPair + gaitWave) & 1) != 0;
        int gaitSlot = limbPairs > 0 ? gaitPair * 2 + (gaitRightSide ? 1 : 0) : 0;
        XMFLOAT3 trailMotion{0.0f, 0.0f, 0.0f};
        if (monsterPresentation_.trail.size() >= 4) {
            trailMotion = Sub3(monsterPresentation_.trail.front(), monsterPresentation_.trail[std::min<size_t>(3, monsterPresentation_.trail.size() - 1)]);
        }
        float trailMotionLen = Length3(trailMotion);
        XMFLOAT3 trailMotionDir = Normalize3(trailMotion, forward);
        float trailMotionAmount = Clamp01(trailMotionLen / std::max(0.05f, maze.TileMinimum() * 0.22f));
        struct LimbReachTarget {
            XMFLOAT3 target;
            XMFLOAT3 normal;
            float swing;
        };
        auto limbReachTarget = [&](XMFLOAT3 root, XMFLOAT3 sideDir, XMFLOAT3 tangentDir, XMFLOAT3 upDir, int limbId, float upperLen, float lowerLen) {
            MonsterLimbAnchor& anchor = monsterPresentation_.limbAnchors[static_cast<size_t>(limbId)];
            float maxReach = std::max(0.18f, upperLen + lowerLen);
            float reachVariance = Rand01(limbId * 17 + 3, 787, sessionRuntime_.runtimeSeed);
            float detachReach = maxReach * (0.88f + reachVariance * 0.12f);
            float emergencyDetachReach = maxReach * (1.15f + reachVariance * 0.08f);
            float plantSlack = maxReach * (0.12f + Rand01(limbId * 19 + 5, 797, sessionRuntime_.runtimeSeed) * 0.08f);
            float now = timeRuntime_.time;
            float roamScramble = SmoothStep(0.0f, 1.0f, Clamp01(world.monsterRoamBurstTimer / 1.05f)) * (1.0f - monsterPresentation_.headChaseBlend * 0.65f);
            float chaseScramble = SmoothStep(0.18f, 1.0f, monsterPresentation_.headChaseBlend);
            float scramble = Clamp01(roamScramble + chaseScramble * 0.72f);
            float lead = maxReach * Lerp(0.04f, 0.15f, trailMotionAmount) * Lerp(0.82f, 1.12f, scramble);
            XMFLOAT3 strideLead = Scale3(trailMotionDir, lead);
            XMFLOAT3 desiredRoot = Add3(root, strideLead);
            SurfaceContact restContact = contactPoint(desiredRoot, sideDir, tangentDir, upDir, limbId, anchor.retargetCount, maxReach);
            if (!anchor.planted && anchor.swingDuration <= 0.001f) {
                anchor.swingDuration = (0.50f + Rand01(limbId * 23 + 7, 803, sessionRuntime_.runtimeSeed) * 0.34f) * Lerp(1.0f, 0.62f, scramble);
            }
            if (!anchor.planted && Length3(Sub3(anchor.swingTo, anchor.swingFrom)) <= 0.001f) {
                anchor.anchor = restContact.point;
                anchor.anchorNormal = restContact.normal;
                anchor.swingFrom = anchor.anchor;
                anchor.swingTo = anchor.anchor;
                anchor.swingFromNormal = anchor.anchorNormal;
                anchor.swingToNormal = anchor.anchorNormal;
                anchor.planted = true;
                recordHandprint(anchor.anchor, anchor.anchorNormal, limbId, anchor.retargetCount);
            }
            if (anchor.planted) {
                float stretch = Length3(Sub3(anchor.anchor, root));
                float impatient = std::sin(now * (0.31f + Rand01(limbId, 809, sessionRuntime_.runtimeSeed) * 0.19f) + Rand01(limbId, 811, sessionRuntime_.runtimeSeed) * kPi * 2.0f);
                bool gaitTurn = requiredLimbAnchors <= 1 || limbId == gaitSlot;
                bool emergencyReplant = stretch > emergencyDetachReach;
                float restDrift = Length3(Sub3(anchor.anchor, restContact.point));
                float restNormalChange = 1.0f - Clamp01(Dot3(Normalize3(anchor.anchorNormal, upDir), Normalize3(restContact.normal, upDir)));
                bool restNeedsUpdate = restDrift > maxReach * Lerp(0.55f, 0.36f, std::max(scramble, trailMotionAmount)) ||
                    restNormalChange > Lerp(0.72f, 0.44f, scramble);
                bool wantsReplant = emergencyReplant || (gaitTurn && (stretch > detachReach ||
                    restNeedsUpdate ||
                    (stretch > detachReach - plantSlack * Lerp(1.0f, 1.45f, scramble) && impatient > Lerp(0.92f, 0.46f, scramble))));
                if (wantsReplant) {
                    anchor.planted = false;
                    anchor.swingFrom = anchor.anchor;
                    anchor.swingFromNormal = anchor.anchorNormal;
                    anchor.retargetCount += 1;
                    SurfaceContact contact = restNeedsUpdate && !emergencyReplant
                        ? restContact
                        : contactPoint(desiredRoot, sideDir, tangentDir, upDir, limbId, anchor.retargetCount, maxReach);
                    anchor.swingTo = contact.point;
                    anchor.swingToNormal = contact.normal;
                    anchor.swingStart = now;
                    anchor.swingDuration = (0.44f + Rand01(limbId + anchor.retargetCount * 13, 811, sessionRuntime_.runtimeSeed) * 0.34f) * Lerp(1.0f, 0.58f, scramble);
                } else {
                    return LimbReachTarget{anchor.anchor, anchor.anchorNormal, 0.0f};
                }
            }
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
