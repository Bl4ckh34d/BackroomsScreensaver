R"(
        float ceilingCornerSW = floorMergeSW * (1.0 - smoothstep(0.18, 0.58, length(float2(uv.x, uv.y) * float2(1.0, 1.0)) - ceilingSeamNoise));
        float ceilingCornerSE = floorMergeSE * (1.0 - smoothstep(0.18, 0.58, length(float2(1.0 - uv.x, uv.y) * float2(1.0, 1.0)) + ceilingSeamNoise));
        float ceilingSeamShape = max(max(ceilingWetN, ceilingWetS), max(ceilingWetW, ceilingWetE));
        float ceilingCornerShape = max(max(ceilingCornerNW, ceilingCornerNE), max(ceilingCornerSW, ceilingCornerSE));
        float ceilingNeighborShape = max(ceilingSeamShape * (0.30 + ceilingSeamBreak * 0.16),
            ceilingCornerShape * (0.72 + ceilingSeamBreak * 0.26));
        ceilingNeighborShape *= 1.0 - smoothstep(0.82, 0.98,
            Fbm3(float3(uv * 13.0 + seed * 23.0, seed * 79.0))) * 0.18;
        ceilingShape = max(ceilingShape, ceilingNeighborShape);
        float2 compactOffset = float2(Hash21(float2(seed * 109.0, 71.0)),
                                      Hash21(float2(seed * 113.0, 73.0))) - 0.5;
        compactOffset *= 0.22;
        float2 compactQ = mergedD - compactOffset;
        float compactAngle = seed * 4.7 + Hash21(float2(seed * 127.0, 79.0)) * 2.2;
        float2 compactRot = float2(cos(compactAngle), sin(compactAngle));
        compactQ = float2(dot(compactQ, compactRot), dot(compactQ, float2(-compactRot.y, compactRot.x)));
        compactQ *= float2(1.04 + Hash21(float2(seed * 131.0, 83.0)) * 0.90,
                           0.82 + Hash21(float2(seed * 137.0, 89.0)) * 0.56);
        float compactRadius = 0.18 + Hash21(float2(seed * 139.0, 97.0)) * 0.17;
        float compactNoise = (Fbm3(float3((warpedUv + compactOffset) * 11.0 + seed * 7.0, seed * 101.0)) - 0.5) * 0.105;
        float compactMask = 1.0 - smoothstep(compactRadius, compactRadius + 0.145, length(compactQ) + compactNoise);
        float compactCore = 1.0 - smoothstep(compactRadius * 0.58, compactRadius + 0.090, length(compactQ) + compactNoise * 0.62);
        float compactBreak = 1.0 - smoothstep(0.76, 0.96,
            Fbm3(float3(warpedUv * (12.0 + seed * 3.0) + seed * 17.0, seed * 107.0))) * 0.16;
        ceilingShape = lerp(ceilingShape, max(ceilingShape * compactMask, compactCore * 0.68) * compactBreak, ceilingCompact);
        float sidePick = lerp(floor(seed * 4.0), encodedSide, encodedEdge);
        float edgeAway = uv.y;
        float edgeAlong = uv.x;
        if (sidePick >= 1.0 && sidePick < 2.0)
        {
            edgeAway = 1.0 - uv.y;
            edgeAlong = 1.0 - uv.x;
        }
        else if (sidePick >= 2.0 && sidePick < 3.0)
        {
            edgeAway = uv.x;
            edgeAlong = 1.0 - uv.y;
        }
        else if (sidePick >= 3.0)
        {
            edgeAway = 1.0 - uv.x;
            edgeAlong = uv.y;
        }
        float edgeNoise = Fbm3(float3(edgeAlong * 5.2 + seed * 4.3, edgeAway * 3.4 - seed * 2.1, seed * 43.0));
        float edgeFine = Fbm3(float3(edgeAlong * 19.0 - seed * 7.0, edgeAway * 14.0 + seed * 3.0, seed * 71.0));
        float edgeReach = 0.28 + seed * 0.30 + (edgeNoise - 0.5) * 0.18;
        float edgeFront = 1.0 - smoothstep(edgeReach, edgeReach + 0.13 + abs(edgeNoise - 0.5) * 0.10, edgeAway);
        float edgeTaper = smoothstep(0.02, 0.18, edgeAlong) * smoothstep(0.98, 0.74, edgeAlong);
        float edgeBreakup = smoothstep(0.18, 0.76, edgeNoise + (edgeFine - 0.5) * 0.24);
        float edgeThreads = smoothstep(0.80, 0.98, edgeFine) *
            (1.0 - smoothstep(edgeReach * 0.80, edgeReach + 0.26, edgeAway)) * edgeTaper;
        float edgeShape = max(edgeFront * edgeTaper * edgeBreakup, edgeThreads * 0.72);
        ceilingShape = max(ceilingShape, edgeShape * edgeMode);
        float horizontal = lerp(ceilingShape, floorShape, floorSurface);
        float wallFlowY = uv.y;
        float wallSource = wallFromCeiling;
        float wallWaterCore = 0.0;
        float wallWaterHalo = 0.0;
        float wallWaterSoak = 0.0;
        float wallBottomSoak = 0.0;
        float wallCardSideFade = smoothstep(0.014, 0.095, uv.x) * (1.0 - smoothstep(0.905, 0.990, uv.x));
        float wallEndFade = 1.0 - smoothstep(0.995, 1.012, wallFlowY);
        float wallWaterDebugActive = step(1.0, gTransition0.w);
        float wallWaterDebugPhase = frac(gTransition0.w);
        float wallWaterBaseAge = lerp(9.0, wallWaterDebugPhase * 8.5, wallWaterDebugActive);
        [loop]
        for (int wf = 0; wf < 11; ++wf)
        {
            float fi = (float)wf;
            float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
            float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
            float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
            float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 2.0);
            float clusterId = floor(fmod(fi + floor(seed * 19.0), clusterCount));
            float uniformCenter = 0.070 + ((fi + 0.25 + r0 * 0.50) / 11.0) * 0.86;
            float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
            float center = clamp(lerp(uniformCenter, clusterCenter + (r1 - 0.5) * 0.14, 0.46 + r2 * 0.22), 0.045, 0.955);
            float enabled = step(0.18, Hash21(float2(seed * 109.0 + fi, 41.0)));
            float len = 1.035 + r1 * 0.105;
            float flowDelay = r0 * 1.30 + fi * (0.045 + r2 * 0.030);
            float flowAge = max(0.0, wallWaterBaseAge - flowDelay);
            float speedPhase = flowAge * (0.58 + r2 * 0.74) + seed * 19.0 + fi * 1.73;
            float flowClock = max(0.0, flowAge * (1.08 + r1 * 0.28) +
                sin(speedPhase) * (0.10 + r0 * 0.09) +
                sin(speedPhase * 2.11 + r2 * 5.0) * 0.030);
            float flowGrow = smoothstep(0.0, 1.0, saturate(flowClock * (0.42 + r1 * 0.14)));
            float dynamicLen = lerp(0.075 + r2 * 0.055, len, flowGrow);
            float flowReady = smoothstep(0.02, 0.34 + r0 * 0.24, flowAge);
            float width = 0.0040 + r2 * 0.0075;
            float wander = (r1 - 0.5) * wallFlowY * (0.020 + r2 * 0.028) +
                (Fbm3(float3(wallFlowY * 5.2 + fi, seed * 11.0, 2.0)) - 0.5) * (0.007 + r0 * 0.010);
            float du = uv.x - center - wander;
            float trailGate = (1.0 - smoothstep(dynamicLen, dynamicLen + 0.045, wallFlowY)) *
                smoothstep(0.000, 0.035 + r0 * 0.025, wallFlowY);
            float breakNoise = smoothstep(0.16, 0.70,
)"
