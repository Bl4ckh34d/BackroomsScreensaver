R"(
                SelectBloodRevealSlot(gBlood3, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood4, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood5, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood6, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood7, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood8, 1.0, input.worldPos.xz, time, animMask, leakAge);
            }
        }
        else
        {
            SelectBloodRevealSlot(float4(gBlood0.xy, gBlood0.z, gBlood1.y), gBlood0.w, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood2, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood3, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood4, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood5, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood6, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood7, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood8, 1.0, input.worldPos.xz, time, animMask, leakAge);
        }
        if (animMask <= 0.0005)
        {
            discard;
        }
        float2 bloodUvMeters = BloodUvWorldMeters(rawUv, input.worldPos);
        float alpha = 0.0;
        float bloodQuality = clamp(gBlood1.w, 0.25, 1.0);
        float requestedStreams = clamp(round(gBlood1.x), 4.0, 32.0);
        float streamBudget = saturate(bloodQuality * 1.15);
        float streamCount = clamp(round(lerp(8.0, requestedStreams, streamBudget)), 6.0, 28.0);
        float floorStreamCount = wallLeakSurface > 0.5
            ? streamCount
            : clamp(round(streamCount * lerp(0.36, 0.74, bloodQuality)), 2.0, streamCount);
        float ceilingStreamCount = wallLeakSurface > 0.5
            ? streamCount
            : clamp(round(streamCount * lerp(0.32, 0.70, bloodQuality)), 2.0, streamCount);
        float canvasStreamCount = clamp(round(lerp(4.0, 12.0, bloodQuality)), 4.0, 12.0);
        float streamWidthScale = max(0.10, gBlood1.z) * lerp(1.55, 1.0, bloodQuality);
        static const bool highBloodDetail = BRM_ENABLE_HIGH_BLOOD != 0;


        if (wallMask > 0.45)
        {
            float u = bloodUv.x;
            float y = bloodUv.y;
            float wallLeakRun = wallLeakSurface;
            float wallStreamWidthScale = streamWidthScale * 1.16;
            float streams = 0.0;
            float streamAccum = 0.0;
            float sdfWet = 0.0;
            float sdfCore = 0.0;
            float diffuseSoak = 0.0;
            float beads = 0.0;
            float topSource = 0.0;
            [loop]
            for (int i = 0; i < 32; ++i)
            {
                float fi = (float)i;
                if (fi >= streamCount) break;
                float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                if (highBloodDetail)
                {
                    densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                }
                float densityBand = densityBase;
                float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                float streamDelay = r0 * 4.6 + fi * (0.05 + r2 * 0.18) +
                    Hash21(float2(seed * 251.0 + fi, 157.0)) * 1.7;
                streamDelay *= lerp(1.0, 0.62, waterLiquid);
                float streamAge = max(0.0, leakAge - streamDelay);
                float speedPhase = streamAge * (0.62 + r2 * 0.54) + seed * 17.0 + fi * 2.13;
                float flowAge = max(0.0, streamAge * (0.90 + r1 * 0.18) +
                    sin(speedPhase) * (0.10 + r0 * 0.08) +
                    sin(speedPhase * 2.17 + r2 * 6.0) * 0.032);
                flowAge *= lerp(1.0, 1.34, waterLiquid);
                float sourceReady = smoothstep(0.10 + r2 * 0.55, 0.90 + r1 * 1.05, flowAge);
                float streamGrow = smoothstep(0.0, 1.0,
                    saturate((flowAge - (0.20 + r2 * 0.75) * lerp(1.0, 0.68, waterLiquid)) *
                    (0.052 + r1 * 0.055) * lerp(1.0, 1.24, waterLiquid)));
                float reachRoll = Hash21(float2(seed * 197.0 + fi, 109.0));
                float partialReach = 0.30 + Hash21(float2(seed * 211.0 + fi, 113.0)) * 0.52;
                float fullReach = 1.02 + Hash21(float2(seed * 223.0 + fi, 127.0)) * 0.18;
                float stopLen = lerp(partialReach, fullReach, step(0.48 + r1 * 0.22, reachRoll));
                float initialLen = 0.006 + r0 * 0.036;
                float len = saturate(lerp(initialLen, stopLen, streamGrow));
                float width = (0.0026 + r2 * 0.0054) * wallStreamWidthScale;
)"
