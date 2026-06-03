R"(
        float dist = length(input.worldPos - cam);
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if (input.material > 10.50 && input.material < 10.90)
    {
        float ndv = saturate(dot(N, V));
        float rim = pow(1.0 - ndv, 2.2);
        float hot = 0.82 + pow(ndv, 0.45) * 0.95 + rim * 0.34;
        float flutter = 0.96 + 0.04 * sin(time * 6.2 + input.material * 19.0);
        float bloodMaterial = smoothstep(0.79, 0.83, frac(input.material));
        float3 fireHot = lerp(float3(8.8, 0.28, 0.075), float3(5.6, 0.025, 0.012), bloodMaterial);
        float3 fireRim = lerp(float3(3.8, 0.065, 0.018), float3(1.9, 0.006, 0.003), bloodMaterial);
        float3 color = fireHot * hot * flutter;
        color += fireRim * rim;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 1.8);
        float fogBlock = saturate(fog * gFog0.z * 1.55);
        float fogVisibility = pow(1.0 - fogBlock, 2.7);
        color *= fogVisibility;
        return float4(saturate(ApplyPost(color) + color * 0.32 * fogVisibility), fogVisibility);
    }

    if (materialId > 8.5 && materialId < 9.5 && frac(input.material) > 0.5)
    {
        float seed = frac(input.material * 13.17);
        float grain = Fbm3(input.worldPos * float3(12.0, 18.0, 12.0) + 2.3);
        float stain = Fbm3(input.worldPos * float3(4.0, 7.0, 4.0) + 18.0);
        float ridge = Fbm3(input.worldPos * float3(38.0, 24.0, 38.0) + 41.0);
        float3 worldN = normalize(N + T * (grain - 0.5) * 0.10 + B * (ridge - 0.5) * 0.055);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 bone = float3(0.82, 0.80, 0.72);
        bone += (grain - 0.5) * float3(0.085, 0.078, 0.060);
        bone -= smoothstep(0.54, 0.88, stain) * float3(0.10, 0.095, 0.075);
        float floorFacing = saturate(abs(N.y));
        float greyish = smoothstep(0.08, 0.34, seed) * (1.0 - smoothstep(0.34, 0.46, seed));
        float yellowed = smoothstep(0.62, 0.90, seed);
        bone = lerp(bone, bone * float3(0.84, 0.86, 0.84), greyish * 0.32 * floorFacing);
        bone = lerp(bone, bone * float3(1.06, 0.96, 0.78), yellowed * 0.42 * floorFacing);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        float spec = pow(facing, 34.0) * 0.22 * (flashlight + sparkLight * 0.45);
        float dist = length(input.worldPos - cam);
        float3 color = bone * (gLighting0.z * 0.34 + flashlight * 1.12 + overhead * 0.26 + sparkLight * 0.80);
        color += float3(0.92, 0.86, 0.70) * spec;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 34.5 && materialId < 35.5)
    {
        float pageCode = frac(input.material);
        float blankPage = 1.0 - step(0.001, pageCode);
        float slot = min(127.0, floor(saturate(pageCode) * 128.0));
        float2 pageUv = saturate(rawUv);
        float4 base = gLoosePages.SampleBias(gSampler, float3(pageUv, slot), -0.35);
        float grain = Fbm3(input.worldPos * float3(14.0, 20.0, 14.0) + slot * 0.37);
        float ridge = Fbm3(input.worldPos * float3(36.0, 22.0, 36.0) + slot * 1.7);
        float edge = max(max(1.0 - smoothstep(0.0, 0.045, rawUv.x), 1.0 - smoothstep(0.0, 0.045, 1.0 - rawUv.x)),
                         max(1.0 - smoothstep(0.0, 0.045, rawUv.y), 1.0 - smoothstep(0.0, 0.045, 1.0 - rawUv.y)));
        float ruleLine = (1.0 - smoothstep(0.006, 0.020, abs(frac(pageUv.y * 28.0 + 0.08) - 0.08))) *
            smoothstep(0.05, 0.14, pageUv.x) * (1.0 - smoothstep(0.86, 0.98, pageUv.x));
        float margin = (1.0 - smoothstep(0.004, 0.018, abs(pageUv.x - 0.175))) *
            smoothstep(0.05, 0.20, pageUv.y) * (1.0 - smoothstep(0.88, 0.98, pageUv.y));
        float3 blankColor = float3(0.88, 0.865, 0.79) + (grain - 0.5) * float3(0.060, 0.055, 0.042);
        blankColor = lerp(blankColor, blankColor * float3(0.77, 0.80, 0.86), ruleLine * 0.34);
        blankColor = lerp(blankColor, blankColor * float3(0.82, 0.78, 0.74), margin * 0.24);
        base = lerp(base, float4(saturate(blankColor), 1.0), blankPage);
        float3 paperN = normalize(N + T * (grain - 0.5) * 0.018 + B * (ridge - 0.5) * 0.010);
        float3 worldN = normalize(lerp(N, paperN, 0.42));
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN, materialId);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float3 paperLift = float3(0.84, 0.82, 0.72) * (0.030 + grain * 0.014);
        float3 imageColor = saturate(lerp(base.rgb, base.rgb * (0.96 + grain * 0.035) + paperLift, 0.035));
        imageColor = lerp(imageColor, imageColor * float3(0.86, 0.83, 0.75), edge * 0.10);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        float hi = max(base.r, max(base.g, base.b));
        float lo = min(base.r, min(base.g, base.b));
        float chroma = hi - lo;
        float glossyCover = smoothstep(0.18, 0.38, chroma) * step(0.72, Hash21(float2(slot, 7.7)));
        float spec = (pow(facing, 34.0) * 0.08 + pow(facing, 120.0) * 0.30 * glossyCover) *
            (flashlight + sparkLight * 0.45 + exitGlow * 0.62);
        float dist = length(input.worldPos - cam);
        float3 color = imageColor * (gLighting0.z * 0.52 + flashlight * 1.08 + overhead * 0.38 + sparkLight * 0.78);
        color += imageColor * exitGreen * 1.18;
        color += float3(0.96, 0.90, 0.76) * spec;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
)"
