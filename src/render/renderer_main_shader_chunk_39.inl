R"(
    float3 diffuseColor = base.rgb * (1.0 - metallic);
    float3 specColor = lerp(float3(0.035, 0.035, 0.035), base.rgb, metallic);
    float ao = lerp(0.58, 1.0, aoMap);
    float3 color = diffuseColor * (ambient + flashlight + sparkLight) * ao;
    color += diffuseColor * overheadColor * ao;
    color += diffuseColor * exitGreen * ao;
    float exitDarkTrimReflect = step(9.5, materialId) * (1.0 - step(10.5, materialId));
    color += exitGreen * exitDarkTrimReflect * 0.045 * ao;
    color = lerp(color, min(color, diffuseColor * (ambient + overhead * 0.18 + flashlight * 0.10 + sparkLight * 0.12) * ao), doorLightBlock);
    float3 toLight = normalize(gShadow0.xyz - input.worldPos);
    float specFacing = saturate(dot(reflect(-toLight, worldN), V));
    float gloss = 1.0 - roughness;
    float fresnelBase = pow(1.0 - saturate(dot(worldN, V)), 5.0);
    float surfaceSpec = (pow(specFacing, lerp(16.0, 170.0, gloss)) * (0.06 + gloss * 0.74) +
        fresnelBase * (0.016 + gloss * 0.075)) *
        (flashlight + sparkLight * 0.5 + exitGlow * 0.45);
    if (materialId > 25.5 && materialId < 26.5)
    {
        float fresnel = pow(1.0 - saturate(dot(worldN, V)), 2.0);
        surfaceSpec += (pow(specFacing, 120.0) * 0.34 + fresnel * 0.11) * (flashlight + sparkLight * 0.45);
        color += base.rgb * (flashlight * 0.22);
    }
    color += specColor * surfaceSpec * (1.0 - doorLightBlock) * lerp(0.70, 1.0, aoMap);
    color = lerp(color, min(color, diffuseColor * (ambient + overhead * 0.14 + flashlight * 0.08 + sparkLight * 0.08) * ao), doorLightBlock);
    if (materialId > 1.5 && materialId < 2.5)
    {
        color += base.rgb * 0.035 * saturate(gLighting0.z * 12.0) * (1.0 - saturate(gTransition0.z));
    }
    if (materialId > 6.5 && materialId < 7.5)
    {
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        float signPulse = 0.94 + 0.06 * sin(time * 5.7);
        color = base.rgb * ((0.28 + flashlight * 0.2) * nonFleshLight + flashlight * 0.12) +
            base.rgb * 2.8 * signPulse * nonFleshLight;
    }
    color *= 1.0 - CornerAO(input.worldPos, worldN);
    float eyeGlow = 0.0;
    if (materialId > 3.5 && materialId < 4.5)
    {
        eyeGlow = saturate((base.r - max(base.g, base.b)) * 3.5);
        color += base.rgb * eyeGlow * 2.4;
    }

    float fogBlock = SceneFogBlock(dist, input.worldPos, 1.0);
    if (materialId > 25.5 && materialId < 26.5)
    {
        fogBlock *= 0.74;
    }
    color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
    float3 posted = ApplyPost(color);
    if (materialId > 3.5 && materialId < 4.5)
    {
        float eyeFogVisibility = pow(1.0 - saturate(fogBlock * 1.55), 2.7);
        eyeGlow *= eyeFogVisibility;
        float death = saturate(gPost0.w);
        float eyeHold = smoothstep(0.48, 0.72, death) * (1.0 - smoothstep(0.92, 1.0, death));
        posted += float3(eyeGlow * 1.7, eyeGlow * 0.06, eyeGlow * 0.025);
        posted += float3(eyeGlow * 4.2, eyeGlow * 0.12, eyeGlow * 0.04) * eyeHold;
    }
    return float4(saturate(posted), base.a);
}
)"
