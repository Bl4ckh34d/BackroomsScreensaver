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
