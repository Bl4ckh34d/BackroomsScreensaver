R"(
        return gDoorFrameNormalHeight.SampleBias(gSampler, uv, mipBias);
    }
    return gNormalHeight.SampleBias(gSampler, MaterialUV(uv, material), mipBias);
}

float2 ReliefParallaxUv(float2 rawUv, float material, float3 viewTS, float scale, float maxLayers)
{
    if (scale <= 0.0001)
    {
        return rawUv;
    }
    viewTS.z = max(viewTS.z, 0.16);
    float layers = lerp(maxLayers, maxLayers * 0.45, saturate(viewTS.z));
    float2 stepUv = (viewTS.xy / viewTS.z) * (scale / max(layers, 1.0));
    float2 uv = rawUv;
    float layerDepth = 1.0 / max(layers, 1.0);
    float currentDepth = 0.0;
    float mapDepth = 1.0 - SampleMaterialNormalHeight(uv, material, 0.0).a;
    [loop]
    for (int i = 0; i < 12; ++i)
    {
        if ((float)i >= layers || currentDepth >= mapDepth)
        {
            break;
        }
        uv -= stepUv;
        currentDepth += layerDepth;
        mapDepth = 1.0 - SampleMaterialNormalHeight(uv, material, 0.0).a;
    }
    return uv;
}

float3 BackroomsBaseColor(float3 base, float materialId)
{
    float luma = dot(base, float3(0.299, 0.587, 0.114));
    if (materialId < 0.5)
    {
        float3 classicWall = float3(0.94, 0.74, 0.29) * (0.68 + luma * 0.68);
        return saturate(lerp(base * 1.08, classicWall, 0.14));
    }
    if (materialId > 0.5 && materialId < 1.5)
    {
        float3 classicCarpet = float3(0.74, 0.62, 0.34) * (0.70 + luma * 0.58);
        return saturate(lerp(base * 1.10, classicCarpet, 0.34));
    }
    if (materialId > 1.5 && materialId < 2.5)
    {
        float3 classicCeiling = float3(0.92, 0.76, 0.30) * (0.78 + luma * 0.42);
        return saturate(lerp(base * float3(1.06, 1.00, 0.76), classicCeiling, 0.72));
    }
    return base;
}

void UnderlyingSurface(float3 worldPos, float3 normal, out float materialId, out float2 uv)
{
    float wallScale = max(gTexture0.x, 0.20);
    float floorScale = max(gTexture0.y, 0.20);
    float explicitCeilingScale = gTexture0.z;
    float2 ceilingScale = explicitCeilingScale > 0.001
        ? max(float2(explicitCeilingScale, explicitCeilingScale), float2(0.20, 0.20))
        : max(gMaze0.zw * 2.0, float2(0.20, 0.20));
    if (normal.y > 0.55)
    {
        materialId = 1.0;
        uv = worldPos.xz / floorScale;
    }
    else if (normal.y < -0.55)
    {
        materialId = 2.0;
        uv = (worldPos.xz - gMaze0.xy) / ceilingScale;
    }
    else if (abs(normal.z) >= abs(normal.x))
    {
        materialId = 0.0;
        uv = -float2(worldPos.x, worldPos.y) / wallScale;
    }
    else
    {
        materialId = 0.0;
        uv = -float2(worldPos.z, worldPos.y) / wallScale;
    }
}

float3 UnderlyingSurfaceColor(float3 worldPos, float3 normal, out float aoMap)
{
    float materialId = 0.0;
    float2 surfaceUv = float2(0.0, 0.0);
    UnderlyingSurface(worldPos, normal, materialId, surfaceUv);
    float mipBias = materialId > 0.5 && materialId < 1.5 ? 1.15 : 0.45;
    float4 base = SampleMaterialAlbedo(surfaceUv, materialId, mipBias);
    float4 pbr = SampleMaterialProps(surfaceUv, materialId, mipBias);
    aoMap = saturate(pbr.r);
    return BackroomsBaseColor(base.rgb, materialId);
}

)"
