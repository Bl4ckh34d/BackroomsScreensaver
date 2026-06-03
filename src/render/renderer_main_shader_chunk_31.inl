R"(
                Fbm3(float3(uv.x * 28.0 + fi * 1.7, wallFlowY * 16.0, seed * 47.0)) + r2 * 0.18);
            float continuousFlow = lerp(breakNoise, max(breakNoise, 0.58 + r2 * 0.18),
                smoothstep(0.34, 0.96, wallFlowY) * flowGrow);
            float gravityWidth = width * (0.72 + smoothstep(0.22, 0.92, wallFlowY) * (0.36 + r1 * 0.44));
            float coreTrail = exp(-(du * du) / max(0.000012, gravityWidth * gravityWidth)) *
                trailGate * continuousFlow * enabled * wallFromCeiling * flowReady * (0.62 + r0 * 0.55);
            float haloTrail = exp(-(du * du) / max(0.000035, gravityWidth * gravityWidth * 18.0)) *
                trailGate * enabled * wallFromCeiling * flowReady * (0.35 + r1 * 0.30);
            float floorContact = exp(-(du * du) / max(0.000035, gravityWidth * gravityWidth * 22.0)) *
                smoothstep(0.82, 0.998, wallFlowY) * flowGrow * enabled * wallFromCeiling * flowReady;
            float sourceWidth = width * (3.8 + r1 * 2.5);
            float sourcePool = exp(-(du * du) / max(0.00005, sourceWidth * sourceWidth)) *
                (1.0 - smoothstep(0.060 + r2 * 0.045, 0.185 + r2 * 0.060, wallFlowY)) *
                enabled * wallFromCeiling * flowReady * (0.32 + r0 * 0.38);
            wallWaterCore = max(wallWaterCore, max(coreTrail, floorContact * 0.32));
            wallWaterHalo = max(wallWaterHalo, max(max(haloTrail * 0.72, sourcePool), floorContact * 0.62));
            wallWaterSoak += saturate(haloTrail * 0.36 + sourcePool * 0.24 + floorContact * 0.12);
        }
        wallWaterSoak = smoothstep(0.18, 1.05, wallWaterSoak) *
            smoothstep(0.10, 0.82, broad + wallWaterHalo * 0.38) *
            wallCardSideFade * wallEndFade;
        wallWaterHalo = saturate(max(wallWaterHalo * wallCardSideFade * wallEndFade, wallWaterSoak * 0.58));
        wallWaterCore *= wallCardSideFade * wallEndFade;
        float bottomDist = 1.0 - uv.y;
        float bottomNoise = (Fbm3(float3(uv.x * 9.0 + seed * 7.0, bottomDist * 5.0, seed * 91.0)) - 0.5) * 0.055;
        wallBottomSoak = wallFromFloor * wallCardSideFade *
            (1.0 - smoothstep(0.10 + bottomNoise, 0.34 + bottomNoise, bottomDist)) *
            smoothstep(0.18, 0.78, broad + fine * 0.16);
        float wallWetShape = saturate(max(max(wallWaterHalo * 0.72, wallWaterCore * 1.15) * wallSource,
            wallBottomSoak * 0.72));
        float shape = lerp(horizontal, wallWetShape, vertical);

        float bridgeBorder = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        float bridgeFilm = smoothstep(0.010, 0.075, bridgeBorder) *
            (0.86 + broad * 0.14);
        shape = max(shape, bridgeFilm * floorBridge);
        float edgeAwareBorder = border;
        if (edgeMode > 0.001)
        {
            if (sidePick < 1.0)
            {
                edgeAwareBorder = min(min(uv.x, 1.0 - uv.x), 1.0 - uv.y);
            }
            else if (sidePick < 2.0)
            {
                edgeAwareBorder = min(min(uv.x, 1.0 - uv.x), uv.y);
            }
            else if (sidePick < 3.0)
            {
                edgeAwareBorder = min(min(uv.y, 1.0 - uv.y), 1.0 - uv.x);
            }
            else
            {
                edgeAwareBorder = min(min(uv.y, 1.0 - uv.y), uv.x);
            }
        }
        float mergedFloorBorder = min(
            min(lerp(uv.x, 1.0, floorMergeW), lerp(1.0 - uv.x, 1.0, floorMergeE)),
            min(lerp(uv.y, 1.0, floorMergeS), lerp(1.0 - uv.y, 1.0, floorMergeN)));
        float floorMergeCoverage = saturate(max(floorSeamShape, floorCornerShape) * 1.65);
        float floorEdgeBorder = lerp(edgeAwareBorder, max(edgeAwareBorder, mergedFloorBorder), floorSurface * floorMergeCoverage);
        float borderNoise = (Fbm3(float3(uv * 11.0 + seed * 23.0, seed * 67.0)) - 0.5) * 0.032;
        float floorBorder = smoothstep(0.030 + borderNoise * 0.65, 0.155 + borderNoise * 1.15, floorEdgeBorder);
        float ceilingMergeCoverage = saturate(max(ceilingSeamShape, ceilingCornerShape) * 1.55);
        float ceilingEdgeAwareBorder = lerp(edgeAwareBorder, max(edgeAwareBorder, mergedFloorBorder), ceilingSurface * ceilingMergeCoverage);
        float ceilingSoftBorder = smoothstep(0.014 + borderNoise, 0.165 + borderNoise, ceilingEdgeAwareBorder);
        float ceilingEdgeBorder = smoothstep(0.006 + borderNoise * 0.45, 0.095 + borderNoise * 0.60, ceilingEdgeAwareBorder);
        float ceilingBorder = lerp(ceilingSoftBorder, ceilingEdgeBorder, edgeMode);
        float cardBorder = lerp(ceilingBorder, floorBorder, floorSurface);
        float verticalBorder = smoothstep(0.006, 0.14, border);
        float wallSideBorder = smoothstep(0.006, 0.14, min(uv.x, 1.0 - uv.x));
        verticalBorder = lerp(verticalBorder, wallSideBorder, saturate(wallFromCeiling + wallFromFloor));
        shape *= lerp(lerp(cardBorder, 1.0, floorBridge), verticalBorder, vertical);
        float debugLoopActive = step(1.0, gTransition0.w);
        float debugPhase = frac(gTransition0.w);
        float debugSpread = saturate((debugPhase - 0.05) / 0.70);
        float debugFade = 1.0 - smoothstep(0.88, 0.98, debugPhase);
        float2 debugCenterTile = floor(gMaze1.xy * 0.5) + 0.5;
        float2 debugCenterXZ = gMaze0.xy + debugCenterTile * gMaze0.zw;
        float2 debugTileDelta = (input.worldPos.xz - debugCenterXZ) / max(gMaze0.zw, float2(0.001, 0.001));
        float debugRadius = max(0.80, (max(gMaze1.x, gMaze1.y) - 2.0) * 0.58);
        float debugDist = lerp(length(debugTileDelta * float2(0.82, 0.82)), lerp(uv.y, 1.0 - uv.y, wallFromFloor), vertical);
        float debugEdgeNoise = (fine - 0.5) * 0.13 + (broad - 0.5) * 0.07;
        float debugReveal = (1.0 - smoothstep(debugSpread * debugRadius, debugSpread * debugRadius + 0.42, debugDist + debugEdgeNoise)) * debugFade;
        shape *= lerp(1.0, debugReveal, debugLoopActive);
        float touchdownGrow = smoothstep(0.46, 0.66, debugPhase);
        shape *= lerp(1.0, touchdownGrow, debugLoopActive * floorTouchdown);
        if (shape < 0.045) discard;

        float core = smoothstep(0.13, 0.82, shape);
        core = max(core, wallWaterCore * vertical);
        float rim = smoothstep(0.035, 0.18, shape) * (1.0 - smoothstep(0.38, 0.74, shape));
        float3 wetN = normalize(N + T * (fine - 0.5) * 0.035 + B * (broad - 0.5) * 0.026);
        float flashlight = FlashlightAmount(input.worldPos, wetN);
        float overhead = LocalLampLight(input.worldPos, wetN, time) * gLighting1.x;
)"
