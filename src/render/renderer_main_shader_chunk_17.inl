R"(
        float secondChance = step(0.76, Hash21(float2(seed * 79.0, 19.0)));
        float secondOffset = dripOffset + lerp(-0.18, 0.18, Hash21(float2(seed * 83.0, 23.0)));
        float secondLen = lerp(0.14, 0.44, Hash21(float2(seed * 89.0, 29.0))) * secondChance;
        float secondDrip = 1.0 - smoothstep(0.012, 0.034, abs(p.x - secondOffset));
        secondDrip *= smoothstep(0.00, 0.10, dripY) * (1.0 - smoothstep(secondLen, secondLen + 0.12, dripY));
        float noise = Fbm3(float3(input.worldPos * 18.0 + seed * 31.0));
        float shape = max(spot, max(max(mainDrip * 0.82, secondDrip * 0.58), drop * 0.72));
        float dryBreakup = smoothstep(0.12, 0.64, noise);
        float alpha = saturate(shape * (0.74 + dryBreakup * 0.22));
        if (alpha < 0.055) discard;
        float3 worldN = normalize(N + T * (noise - 0.5) * 0.045 + B * (Fbm3(input.worldPos * 31.0 + seed * 17.0) - 0.5) * 0.035);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float lightEnergy = gLighting0.z * 0.035 + flashlight * 0.92 + overhead * 0.32 + sparkLight * 0.64;
        float3 blood = lerp(float3(0.055, 0.002, 0.001), float3(0.42, 0.014, 0.006), saturate(noise * 1.25));
        float wetSpec = pow(saturate(dot(reflect(-normalize(gShadow0.xyz - input.worldPos), worldN), V)), 68.0) *
            saturate(flashlight + overhead * 0.35 + sparkLight * 0.45);
        float3 color = blood * (0.22 + lightEnergy * 1.10) + float3(0.95, 0.10, 0.045) * wetSpec * 0.34;
        float dist = length(input.worldPos - cam);
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), alpha * 0.72);
    }

    if (input.material > 26.05 && input.material < 26.95)
    {
        float seed = frac(input.material);
        float fiber = Fbm3(input.worldPos * float3(8.0, 11.0, 8.0) + float3(seed * 31.0, 0.0, seed * 17.0));
        float vein = smoothstep(0.74, 0.96, Fbm3(input.worldPos * float3(17.0, 22.0, 17.0) + seed * 53.0));
        float3 worldN = normalize(N + T * (fiber - 0.5) * 0.10 + B * (vein - 0.5) * 0.06);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN, materialId);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float lightEnergy = gLighting0.z * 0.08 + flashlight * 1.08 + overhead * 0.24 + sparkLight * 0.58 + exitGlow * 0.18;
        float3 flesh = lerp(float3(0.12, 0.018, 0.014), float3(0.34, 0.038, 0.025), fiber);
        flesh = lerp(flesh, float3(0.035, 0.005, 0.014), vein * 0.42);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        float wet = pow(facing, 74.0) * saturate(flashlight + sparkLight * 0.45 + overhead * 0.18);
        float3 color = flesh * (0.10 + lightEnergy);
        color += float3(0.70, 0.12, 0.07) * wet * 0.42;
        float dist = length(input.worldPos - cam);
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if ((materialId > 13.5 && materialId < 14.5) || (materialId > 24.5 && materialId < 25.5))
    {
        float waterLiquid = step(24.5, materialId);
        float rawSeed = frac(input.material);
        float canvasSurface = step(0.990, rawSeed);
        float wallLeakSurface = step(0.96, rawSeed) * (1.0 - canvasSurface);
        float centerSeepSurface = step(0.43, rawSeed) * (1.0 - step(0.95, rawSeed)) * (1.0 - canvasSurface);
        float menuCenterSeepSurface = centerSeepSurface * step(0.62, rawSeed) * (1.0 - step(0.70, rawSeed));
        float seed = rawSeed;
        if (canvasSurface > 0.5)
        {
            seed = saturate((rawSeed - 0.990) / 0.009);
        }
        else if (wallLeakSurface > 0.5)
        {
            seed = saturate((rawSeed - 0.965) / 0.025);
        }
        else if (centerSeepSurface > 0.5)
        {
            seed = saturate((rawSeed - 0.43) / 0.52);
        }
        else if (rawSeed >= 0.02 && rawSeed <= 0.42)
        {
            seed = saturate((rawSeed - 0.02) / 0.40);
        }
        float wet = lerp(saturate(gHorror0.y), 1.0, waterLiquid);
        float2 bloodUv = lerp(uv, rawUv, waterLiquid);
        float drips = 0.0;
        float thickness = 0.0;
        float wallMask = saturate(1.0 - abs(N.y));
        float floorMask = smoothstep(0.45, 0.82, N.y);
        float ceilingMask = smoothstep(0.45, 0.82, -N.y);
        float animMask = 0.0;
        float leakAge = 0.0;
        if (waterLiquid > 0.5)
        {
            float waterDebugActive = step(1.0, gTransition0.w);
            float waterDebugPhase = frac(gTransition0.w);
            if (waterDebugActive > 0.5)
            {
                animMask = 1.0;
                leakAge = waterDebugPhase * 54.0;
            }
            else
            {
                SelectBloodRevealSlot(float4(gBlood0.xy, gBlood0.z, gBlood1.y), gBlood0.w, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood2, 1.0, input.worldPos.xz, time, animMask, leakAge);
)"
