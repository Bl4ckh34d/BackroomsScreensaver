R"(
                float rimSideFade = smoothstep(0.010 + edgeJitterU, 0.090 + edgeJitterU, u) *
                    (1.0 - smoothstep(0.910 - edgeJitterU, 0.990 - edgeJitterU, u));
                float rimBreakup = smoothstep(0.16, 0.72, rimNoise + (rimFine - 0.5) * 0.22);
                float delayedRim = (1.0 - smoothstep(rimWidth, rimWidth + 0.070, away)) *
                    rimSideFade * rimBreakup * rimAge;
                float soakFrontNoise = (Fbm3(float3(u * 3.4 + seed * 13.0, away * 1.7, seed * 41.0)) - 0.5) * 0.22 +
                    (Noise3(float3(u * 11.0, away * 5.0, seed * 73.0)) - 0.5) * 0.06;
                float rimFeed = smoothstep(0.04, 0.30, topSource);
                float soakReach = saturate((leakAge - lerp(7.5, 3.8, waterLiquid)) / lerp(34.0, 34.0, waterLiquid));
                float unevenCeilingReach = saturate(soakReach * lerp(1.0, 1.32, waterLiquid) + rimFeed * lerp(0.22, 0.34, waterLiquid) + soakFrontNoise);
                float ceilingFront = 1.0 - smoothstep(unevenCeilingReach,
                    unevenCeilingReach + 0.17 + abs(soakFrontNoise) * 0.10,
                    away);
                float ceilingSoak = smoothstep(lerp(8.0, 5.0, waterLiquid), lerp(31.0, 52.0, waterLiquid), leakAge) *
                    ceilingFront * smoothstep(0.006, 0.058, away) *
                    (1.0 - smoothstep(1.01 + edgeJitterV, 1.11 + edgeJitterV, away)) *
                    cardEdgeFade * organicMask * (0.62 + ceilingNoise * 0.38);
                topSource = saturate(topSource * cardEdgeFade + ceilingSoak * lerp(0.52, 0.74, waterLiquid) +
                    delayedRim * (lerp(0.52, 0.68, waterLiquid) + rimFeed * 0.40));
                alpha = topSource * raggedEdge * ceilingMask * smoothstep(0.0, 0.35, leakAge);
                alpha = smoothstep(lerp(0.014, 0.008, waterLiquid), lerp(0.092, 0.072, waterLiquid), alpha);
                thickness = saturate(topThickness * 0.86 + ceilingSoak * lerp(0.54, 0.78, waterLiquid) +
                    delayedRim * lerp(0.42, 0.58, waterLiquid)) * alpha;
            }
            else
            {
                float centerThickness = 0.0;
                float centerSpeed = lerp(0.024, 0.016, waterLiquid) * lerp(1.0, 3.45, menuCenterSeepSurface);
                float centerRadius = lerp(0.58, 0.74, waterLiquid) * lerp(1.0, 1.20, menuCenterSeepSurface);
                float source = CenterSeepPool(bloodUv, input.worldPos, seed, leakAge, centerSpeed, centerRadius, centerThickness);
                alpha = source * ceilingMask * smoothstep(0.0, lerp(0.65, 1.05, waterLiquid) * lerp(1.0, 0.32, menuCenterSeepSurface), leakAge);
                alpha = smoothstep(lerp(0.024, 0.012, waterLiquid), lerp(0.116, 0.088, waterLiquid), alpha);
                thickness = centerThickness * alpha;
            }
            drips = 0.0;
        }

        float wetAlpha = alpha;
        float wetThickness = thickness;
        float wetDrips = drips;
        alpha = wetAlpha * animMask;
        thickness = wetThickness * animMask;
        drips = wetDrips * animMask;
        if (alpha < lerp(0.045, 0.026, waterLiquid)) discard;

        float2 local = bloodUv * 2.0 - 1.0;
        float2 bulgeSlope = -local * thickness * (0.08 + wet * 0.08);
        bulgeSlope.y += wallMask * drips * (0.030 + wet * 0.028);
        bulgeSlope += (float2(
            Fbm3(float3(bloodUv * 18.0, seed * 47.0)),
            Fbm3(float3(bloodUv.yx * 18.0 + 3.7, seed * 53.0))) - 0.5) * thickness * 0.010;
        float3 B2 = normalize(cross(N, T));
        float3 worldN = normalize(N + T * bulgeSlope.x + B2 * bulgeSlope.y);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN, materialId);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float3 reflectDir = reflect(-toLight, worldN);
        float facing = saturate(dot(reflectDir, V));
        float fresnel = pow(1.0 - saturate(dot(worldN, V)), 4.0);
        float specLight = saturate(flashlight + overhead * 0.28 + sparkLight * 0.52 + exitGlow * 0.16);
        float spec = (pow(facing, 170.0) * 0.96 + pow(facing, 38.0) * 0.13 + fresnel * 0.055) *
            specLight * (0.18 + wet * 0.72) * saturate(alpha + thickness * 0.48);
        float grime = Fbm3(input.worldPos * float3(8.0, 5.0, 8.0) + seed * 31.0);
        float filmAlpha = saturate(alpha * (0.58 + thickness * 0.38 + drips * 0.12 + wet * 0.035));
        if (filmAlpha < lerp(0.045, 0.026, waterLiquid)) discard;
        float lightEnergy = saturate(flashlight * 0.68 + overhead * 0.25 + sparkLight * 0.34 + exitGlow * 0.095 + gLighting0.z * 0.050);
        float3 thinBlood = float3(0.430, 0.0060, 0.0014);
        float3 pooledBlood = float3(0.105, 0.00075, 0.00022);
        float3 blood = lerp(thinBlood, pooledBlood, saturate(thickness * 0.88 + drips * 0.24));
        float3 color = blood * (0.20 + lightEnergy * 0.88) * (0.56 + grime * 0.15);
        color = lerp(color, color * float3(0.46, 0.070, 0.045), drips * 0.20);
        float flashlightWetSpec = pow(facing, 112.0) * flashlight * wet * saturate(alpha + thickness * 0.34);
        color += float3(0.33, 0.010, 0.0035) * spec;
        color += float3(0.78, 0.012, 0.0038) * flashlightWetSpec;
        color += float3(0.060, 0.006, 0.004) * fresnel * specLight * wet * saturate(alpha + thickness * 0.24);
        color += blood * exitGlow * (0.10 + wet * 0.18) * saturate(alpha + thickness * 0.20);
        if (waterLiquid > 0.5)
        {
            float waterCore = saturate(alpha * 0.74 + thickness * 0.26);
            float waterFresnel = pow(1.0 - saturate(dot(worldN, V)), 3.0);
            float waterSpec = spec * (0.20 + waterCore * 0.42) + waterFresnel * specLight * (0.025 + waterCore * 0.055);
            float3 thinTint = float3(0.010, 0.011, 0.010) * (0.34 + lightEnergy * 0.16);
            float3 deepTint = float3(0.0045, 0.0052, 0.0048) * (0.25 + lightEnergy * 0.10);
            float3 clearFilm = lerp(thinTint, deepTint, saturate(thickness * 0.85 + drips * 0.24));
            float3 reflectedLamp = float3(0.25, 0.26, 0.24) * waterSpec;
            color = clearFilm * (0.80 + waterCore * 0.34) + reflectedLamp;
            color += exitGreen * (0.002 + waterCore * 0.009);
            filmAlpha = saturate(alpha * (0.28 + thickness * 0.15 + drips * 0.055));
        }
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.45;
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
)"
