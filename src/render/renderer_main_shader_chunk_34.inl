R"(

    if (materialId > 12.5 && materialId < 13.5)
    {
        float2 d = uv - 0.5;
        float r = length(d);
        float r2 = dot(d, d);
        float variant = frac(input.material);
        float core = exp(-r2 * 86.0);
        float halo = exp(-r2 * 28.0);
        float roundMask = smoothstep(0.47, 0.18, r);
        float alpha = saturate(core * 1.18 + halo * 0.44) * roundMask * (0.70 + variant * 0.62);
        if (alpha < 0.016) discard;
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        if (nonFleshLight < 0.001) discard;
        float3 color = (float3(8.4, 3.7, 0.72) * core + float3(2.8, 0.78, 0.12) * halo) * nonFleshLight;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 1.4);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.24);
        return float4(saturate(ApplyPost(color) + color * 0.18), saturate(alpha));
    }

    if (materialId > 14.5 && materialId < 15.5)
    {
        float variant = frac(input.material);
        float particleFade = smoothstep(0.055, 0.24, variant);
        float3 toLight = input.worldPos - gShadow0.xyz;
        float lightDist = length(toLight);
        float focus = max(0.45, gAir0.x);
        float blur = saturate(abs(lightDist - focus) / (0.62 + lightDist * 0.18)) * saturate(gAir0.y);
        float2 p = uv * 2.0 - 1.0;
        float3 stable = floor(input.worldPos * 2.7 + variant * 31.0);
        float h0 = Hash31(stable + 3.0);
        float h1 = Hash31(stable.yzx + 17.0);
        float h2 = Hash31(stable.zxy + 41.0);
        float strandAngle = h0 * 6.2831853;
        float2 strandDir = float2(cos(strandAngle), sin(strandAngle));
        float2 strandPerp = float2(-strandDir.y, strandDir.x);
        float strandX = dot(p, strandDir);
        float strandY = dot(p, strandPerp);
        float strandTaper = 1.0 - smoothstep(0.42 + h1 * 0.22, 1.05 + h2 * 0.16, abs(strandX));
        float waviness = sin(strandX * (10.0 + h0 * 11.0) + variant * 31.0 + time * 0.05) * (0.020 + h1 * 0.026);
        float hairA = exp(-pow(abs(strandY - waviness), 1.35) * (74.0 + h2 * 86.0)) * strandTaper;
        float hairB = exp(-pow(abs(strandY - 0.075 - waviness * 0.55), 1.28) * (96.0 + h0 * 72.0)) *
            (1.0 - smoothstep(0.25 + h2 * 0.16, 0.92, abs(strandX + 0.14)));
        float hairC = exp(-pow(abs(strandY + 0.060 + waviness * 0.70), 1.32) * (110.0 + h1 * 60.0)) *
            (1.0 - smoothstep(0.18 + h0 * 0.22, 0.86, abs(strandX - 0.10)));
        float clumpBreak = smoothstep(0.18, 0.72, Fbm3(float3(p * (8.0 + h2 * 7.0) + variant * 17.0, variant * 53.0)));
        float hairClump = max(hairA, max(hairB * 0.76, hairC * 0.62)) * lerp(0.54, 1.0, clumpBreak);
        float2 q = p + strandPerp * waviness * 0.45 + strandDir * ((h1 - 0.5) * 0.18) + strandPerp * ((h2 - 0.5) * 0.13);
        float angle = atan2(q.y, q.x);
        float lobesA = sin(angle * (3.0 + floor(h0 * 5.0)) + variant * 38.0 + time * 0.035);
        float lobesB = sin(angle * (7.0 + floor(h1 * 6.0)) + variant * 71.0 - time * 0.026);
        float corner = sin(angle * (11.0 + floor(h2 * 5.0)) + h1 * 19.0);
        float sides = 5.0 + floor(h0 * 6.0);
        float sector = floor((angle + 3.14159265 + variant * 6.2831853) / (6.2831853 / sides));
        float faceted = (Hash21(float2(sector, variant * 43.0)) - 0.5) * 0.34;
        float edge = clamp(0.47 + faceted + lobesA * 0.18 + lobesB * 0.11 + corner * 0.075 + (h1 - 0.5) * 0.16, 0.22, 0.82);
        float r = length(q);
        float blob = smoothstep(edge + 0.08 + blur * 0.19, edge - 0.035 - blur * 0.06, r);
        float biteA = smoothstep(0.18, 0.72, dot(q, strandDir) + h0 * 0.36) *
            smoothstep(0.82, 0.14, abs(dot(q, strandPerp) - (h2 - 0.5) * 0.22));
        float biteB = smoothstep(0.20, 0.78, -dot(q, strandPerp) + h1 * 0.28) *
            smoothstep(0.76, 0.10, abs(dot(q, strandDir) + (h0 - 0.5) * 0.30));
        blob *= 1.0 - saturate(max(biteA * 0.72, biteB * 0.55) * (1.0 - blur * 0.35));
        float raggedEdge = smoothstep(edge + 0.18 + blur * 0.18, edge - 0.02, r) * (1.0 - blob);
        raggedEdge *= smoothstep(0.30, 0.82, Fbm3(float3(q * (11.0 + h2 * 9.0), variant * 61.0)));
        float holes = step(0.60 + blur * 0.18, Hash21(floor((q + variant) * (7.0 + h0 * 8.0))));
        blob *= lerp(1.0, 0.52, holes * (1.0 - blur * 0.45));
        float smear = exp(-pow(abs(dot(q, strandPerp) + (h2 - 0.5) * 0.16), 1.18) * (30.0 + h0 * 48.0)) *
            (1.0 - smoothstep(0.28 + h1 * 0.18, 0.98, abs(dot(q, strandDir) - 0.10)));
        float particleShell = saturate(max(blob, raggedEdge) + smear * 0.72 + hairClump * 0.46);
        float shape = max(blob * 0.48, max(raggedEdge * 0.20, max(smear * 0.38, hairClump * (0.58 + h1 * 0.48))));
        float flecks = lerp(0.78, 1.10, Hash21(floor(q * (10.0 + h1 * 9.0)) + variant * 23.0));
        float asymmetricFalloff = smoothstep(1.20, 0.30, max(abs(p.x) * lerp(0.86, 1.22, h0), abs(p.y) * lerp(0.88, 1.28, h2)));
        shape *= flecks * asymmetricFalloff;
        if (shape < 0.018) discard;

        float flashlight = FlashlightAmount(input.worldPos, N);
        float overhead = LocalLampLight(input.worldPos, N, time) * gLighting1.x;
        float3 doorwayDustLight = ExitSignLight(input.worldPos, N, materialId);
        float doorwayDust = saturate(max(doorwayDustLight.r, max(doorwayDustLight.g, doorwayDustLight.b)) * 0.20);
        float3 lightDir = normalize(gShadow1.xyz);
        float axisDist = max(0.0, dot(toLight, lightDir));
        float outerCos = clamp(gShadow2.z, 0.04, 0.98);
        float coneRadius = max(0.018, axisDist * sqrt(max(0.0, 1.0 - outerCos * outerCos)) / outerCos);
        float radialDist = length(toLight - lightDir * axisDist);
        float centerLine = 1.0 - smoothstep(0.08, 0.84, radialDist / coneRadius);
        float flashlightScale = sqrt(max(0.0, gLighting0.x));
        float centerBoost = (0.76 + centerLine * 0.70) * lerp(0.72, 1.24, saturate(flashlightScale * 0.78));
        float lightFade = smoothstep(0.35, 1.10, lightDist) * (1.0 - smoothstep(gShadow2.y * 0.46, gShadow2.y * 0.82, lightDist));
        float depthFade = 1.0 - smoothstep(gFog0.y * 0.72, gFog0.y, length(input.worldPos - cam));
        float focusAlpha = lerp(1.0, 0.46, blur);
        float particleLight = saturate(flashlight * centerBoost * lightFade + overhead * 0.42 + doorwayDust * 0.92);
        float alpha = shape * particleLight * depthFade * focusAlpha * (0.20 + variant * 0.12) * particleFade * (1.0 - saturate(gTransition0.z));
)"
