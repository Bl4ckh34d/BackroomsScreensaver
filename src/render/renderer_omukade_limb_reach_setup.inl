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
