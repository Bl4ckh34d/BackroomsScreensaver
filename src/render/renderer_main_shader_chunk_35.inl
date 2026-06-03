R"(
        if (alpha < 0.010) discard;
        float3 color = float3(0.72, 0.78, 0.72) * (0.20 + flashlight * (1.08 + centerLine * 0.82));
        color += float3(0.98, 0.90, 0.62) * overhead * (0.50 + particleShell * 0.18);
        color += float3(0.91, 0.965, 1.0) * doorwayDust * (1.45 + particleShell * 0.55);
        color += float3(0.42, 0.48, 0.44) * particleShell * (0.08 + blur * 0.08 + centerLine * 0.08) * flashlightScale;
        float fog = saturate((length(input.worldPos - cam) - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 2.2);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.65);
        return float4(ApplyPost(color), saturate(alpha));
    }

    if (materialId > 17.5 && materialId < 18.5)
    {
        if (frac(input.material) > 0.80)
        {
            float4 panel = gCustomMenu.Sample(gSampler, saturate(uv));
            if (panel.a < 0.025) discard;
            float dist = length(input.worldPos - cam);
            float3 overlayN = normalize(N);
            float flashlight = FlashlightAmount(input.worldPos, overlayN);
            float overhead = LocalLampLight(input.worldPos, overlayN, time) * gLighting1.x;
            float sparkLight = SparkLight(input.worldPos, overlayN);
            float fogVisibility = pow(1.0 - SceneFogBlock(dist, input.worldPos, 0.55), 1.35);
            float markerNoise = Fbm3(float3(uv * 86.0, time * 0.015));
            float3 ink = panel.rgb * (0.76 + markerNoise * 0.18);
            float lit = saturate(gLighting0.z * 0.22 + flashlight * 1.12 + overhead * 0.34 + sparkLight * 0.62);
            ink *= lit;
            return float4(saturate(ApplyPost(ink)), saturate(panel.a * fogVisibility));
        }
        float4 label = gAlbedo.Sample(gSampler, float3(uv, 18.0));
        if (label.a < 0.025) discard;
        float hover = smoothstep(0.35, 0.60, frac(input.material));
        float pulse = lerp(0.72, 0.92 + 0.08 * sin(time * 4.8 + input.worldPos.x * 2.1), hover);
        float3 glow = label.rgb * lerp(1.35 + label.a * 0.80, 4.2 + label.a * 5.8, hover) * pulse;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        float fogVisibility = pow(1.0 - saturate(fog * gFog0.z), 2.2);
        glow *= fogVisibility;
        return float4(saturate(ApplyPost(glow) + glow * lerp(0.02, 0.18, hover)), saturate(label.a * lerp(0.95, 1.55, hover) * fogVisibility));
    }

    if (materialId > 18.5 && materialId < 19.5)
    {
        float strength = smoothstep(0.58, 0.94, frac(input.material));
        float2 p = uv * 2.0 - 1.0;
        float edge = smoothstep(1.18, 0.26, abs(p.x)) * smoothstep(1.18, 0.20, abs(p.y));
        float haze = Fbm3(float3(input.worldPos.xz * 1.35 + p * 0.34, time * 0.045));
        float streak = smoothstep(1.02, 0.16, abs(p.x + (haze - 0.5) * 0.34));
        float dist = length(input.worldPos - cam);
        float fogVisibility = pow(1.0 - SceneFogBlock(dist, input.worldPos, 0.22), 1.12);
        float alpha = edge * lerp(0.030, 0.125, strength) * (0.50 + streak * 0.36) * fogVisibility;
        if (alpha < 0.008) discard;
        float3 color = float3(0.88, 0.95, 1.0) * (2.15 + strength * 4.8) * (0.72 + haze * 0.24);
        return float4(saturate(ApplyPost(color) + color * 0.10), saturate(alpha));
    }

    if (input.material > 20.40 && input.material < 20.95)
    {
        float seed = frac(input.material * 31.41);
        float2 meatUv = input.uv * float2(1.25, 2.65) + float2(seed * 0.37, seed * 0.19 + time * 0.006);
        float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.18));
        float height0 = gNormalHeight.Sample(gSampler, float3(meatUv, 15.0)).a;
        meatUv += (height0 - 0.48) * 0.026 * viewTS.xy / viewTS.z;
        float3 fleshUv = float3(meatUv, 15.0);
        float4 fleshBase = gAlbedo.Sample(gSampler, fleshUv);
        float4 fleshPbr = gMaterialProps.Sample(gSampler, fleshUv);
        float4 fleshNh = gNormalHeight.Sample(gSampler, fleshUv);
        float3 nTex = normalize(fleshNh.xyz * 2.0 - 1.0);
        nTex = normalize(float3(nTex.xy * 0.86, nTex.z));
        float3 gutP = input.worldPos * float3(2.2, 4.8, 2.2) + float3(seed * 19.0, time * 0.13, seed * 43.0);
        float fiber = Fbm3(gutP);
        float veinField = Fbm3(input.worldPos * float3(8.0, 11.0, 8.0) + float3(seed * 71.0, -time * 0.06, seed * 37.0));
        float veinLine = smoothstep(0.78, 0.96, veinField + sin(input.uv.y * 9.0 + input.uv.x * 15.0 + seed * 9.0) * 0.10);
        float pulse = 0.90 + 0.10 * sin(input.uv.y * 4.3 - time * 3.4 + seed * 17.0);
        float3 wetN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);
        wetN = normalize(wetN + T * (fiber - 0.5) * 0.12 + B * (veinField - 0.5) * 0.08);
        float flashlight = FlashlightAmount(input.worldPos, wetN);
        float overhead = LocalLampLight(input.worldPos, wetN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, wetN);
        float3 exitGreen = ExitSignLight(input.worldPos, wetN, materialId);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float lightEnergy = gLighting0.z * 0.10 + flashlight * 1.18 + overhead * 0.24 + sparkLight * 0.70 + exitGlow * 0.22;
        float3 muscle = lerp(fleshBase.rgb * 0.82, fleshBase.rgb * 1.18 + float3(0.11, 0.012, 0.006), fiber);
        float3 vein = float3(0.040, 0.003, 0.012);
        float3 rawColor = lerp(muscle, vein, veinLine * 0.46) * pulse;
        float facing = saturate(dot(reflect(-normalize(gShadow0.xyz - input.worldPos), wetN), V));
        float fresnel = pow(1.0 - saturate(dot(wetN, V)), 2.4);
        float roughness = saturate(fleshPbr.g);
        float gloss = 1.0 - roughness;
        float spec = (pow(facing, lerp(42.0, 150.0, gloss)) * lerp(0.45, 1.35, gloss) + pow(facing, 24.0) * 0.24 + fresnel * 0.20) *
            saturate(flashlight + overhead * 0.35 + sparkLight * 0.55 + exitGlow * 0.30);
        float slime = 0.72 + 0.28 * Fbm3(input.worldPos * 18.0 + time * 0.07);
        float3 color = rawColor * (0.08 + lightEnergy * 1.08) * lerp(0.64, 1.0, saturate(fleshPbr.r));
        color += float3(0.95, 0.42, 0.32) * spec * slime;
        color += float3(0.10, 0.008, 0.006) * fresnel * saturate(lightEnergy) * 0.45;
)"
