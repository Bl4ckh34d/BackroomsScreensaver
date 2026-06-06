    static const char* PostShaderSource() {
        return R"(
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 gViewProj;
    row_major float4x4 gLightViewProj;
    row_major float4x4 gFixtureLightViewProj;
    float4 gCameraPosTime;
    float4 gCameraDirAspect;
    float4 gLighting0;
    float4 gLighting1;
    float4 gFog0;
    float4 gAO0;
    float4 gPost0;
    float4 gPost1;
    float4 gPost2;
};

Texture2D gSceneColor : register(t0);
SamplerState gPostSampler : register(s0);

struct PostVSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PostVSOut PostVS(uint vertexId : SV_VertexID)
{
    PostVSOut o;
    float2 p = float2((vertexId << 1) & 2, vertexId & 2);
    o.uv = p;
    o.pos = float4(p * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return o;
}

float DirtHash(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float LensDirt(float2 uv)
{
    float2 centered = uv - 0.5;
    float vignette = smoothstep(0.18, 0.78, length(centered));
    float fine = DirtHash(floor(uv * 52.0)) * 0.55 + DirtHash(floor(uv * 137.0 + 19.0)) * 0.45;
    float smudgeA = smoothstep(0.36, 0.0, length((uv - float2(0.32, 0.42)) * float2(1.0, 1.8)));
    float smudgeB = smoothstep(0.42, 0.0, length((uv - float2(0.68, 0.58)) * float2(1.5, 0.9)));
    return saturate(vignette * 0.38 + pow(fine, 8.0) * 0.42 + smudgeA * 0.22 + smudgeB * 0.18);
}

float3 BrightPart(float3 c)
{
    float luma = dot(c, float3(0.299, 0.587, 0.114));
    float gate = smoothstep(0.58, 1.0, luma);
    return c * gate;
}

float SceneLuma(float3 c)
{
    return dot(c, float3(0.299, 0.587, 0.114));
}

float3 SampleSceneColor(float2 uv)
{
    return gSceneColor.Sample(gPostSampler, saturate(uv)).rgb;
}

float3 FxaaSceneColor(float2 uv, float2 texel)
{
    float3 rgbM = SampleSceneColor(uv);
    float3 rgbNW = SampleSceneColor(uv + texel * float2(-1.0, -1.0));
    float3 rgbNE = SampleSceneColor(uv + texel * float2( 1.0, -1.0));
    float3 rgbSW = SampleSceneColor(uv + texel * float2(-1.0,  1.0));
    float3 rgbSE = SampleSceneColor(uv + texel * float2( 1.0,  1.0));

    float lumaM = SceneLuma(rgbM);
    float lumaNW = SceneLuma(rgbNW);
    float lumaNE = SceneLuma(rgbNE);
    float lumaSW = SceneLuma(rgbSW);
    float lumaSE = SceneLuma(rgbSE);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    if (lumaMax - lumaMin < max(0.0312, lumaMax * 0.125))
    {
        return rgbM;
    }

    float2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.0078125, 0.0009765625);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, float2(-8.0, -8.0), float2(8.0, 8.0)) * texel;

    float3 rgbA = 0.5 * (
        SampleSceneColor(uv + dir * (1.0 / 3.0 - 0.5)) +
        SampleSceneColor(uv + dir * (2.0 / 3.0 - 0.5)));
    float3 rgbB = rgbA * 0.5 + 0.25 * (
        SampleSceneColor(uv + dir * -0.5) +
        SampleSceneColor(uv + dir *  0.5));
    float lumaB = SceneLuma(rgbB);
    return (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;
}

float4 PostPS(PostVSOut input) : SV_TARGET
{
    uint w;
    uint h;
    gSceneColor.GetDimensions(w, h);
    float2 texel = 1.0 / max(float2(w, h), float2(1.0, 1.0));
    float2 uv = input.uv;
    float danger = saturate(gPost0.z);
    float death = saturate(gPost0.w);
    float time = gCameraPosTime.w;
    float bloomAmount = saturate(gPost1.z);
    float dirtAmount = saturate(gPost1.w);
    float visionFlash = saturate(gPost2.x);
    float fxaa = saturate(gPost2.y);
    float2 motion = clamp(gPost1.xy, float2(-0.045, -0.045), float2(0.045, 0.045));

    float3 color = fxaa > 0.5 ? FxaaSceneColor(uv, texel) : SampleSceneColor(uv);
    float motionWeight = saturate(length(motion) * 42.0);
    [branch]
    if (motionWeight > 0.001)
    {
        float3 blur = color * 0.44;
        blur += gSceneColor.Sample(gPostSampler, saturate(uv - motion * 0.56)).rgb * 0.30;
        blur += gSceneColor.Sample(gPostSampler, saturate(uv + motion * 0.38)).rgb * 0.26;
        color = lerp(color, blur, motionWeight * (0.30 + danger * 0.18));
    }

    [branch]
    if (bloomAmount > 0.001)
    {
        float2 bloomStep = texel * (3.6 + danger * 2.4);
        float3 bloom = BrightPart(color) * 0.60;
        bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 0.85,  0.65)).rgb) * 0.20;
        bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2(-0.85, -0.65)).rgb) * 0.20;
        float dirt = dirtAmount > 0.001 ? LensDirt(uv) * dirtAmount : 0.0;
        color += bloom * bloomAmount * (0.12 + dirt * 0.55);
        color += float3(1.0, 0.92, 0.72) * dirt * bloomAmount * 0.018;
    }
    color *= 1.0 - smoothstep(0.58, 1.04, length((uv - 0.5) * float2(gCameraDirAspect.w, 1.0))) * (0.055 + dirtAmount * 0.035);
    color = lerp(color, float3(1.0, 0.018, 0.0), visionFlash * 0.72);
    color += float3(0.42, 0.0, 0.0) * visionFlash;
    if (death > 0.001)
    {
        float pulseRate = lerp(9.0, 58.0, death);
        float strobeA = frac(time * pulseRate + sin(time * 21.7) * 0.21 + sin(time * 53.1) * 0.11);
        float strobeB = frac(time * (pulseRate * 0.63 + 7.0) + sin(time * 39.3) * 0.19);
        float redFlash = step(0.70 - death * 0.30, strobeA) * smoothstep(0.04, 0.92, death);
        float blackFlash = step(0.76 - death * 0.34, strobeB) * smoothstep(0.12, 0.96, death);
        float whiteFlash = step(0.88 - death * 0.42, frac(strobeA + strobeB * 0.37)) * smoothstep(0.45, 1.0, death);
        color = lerp(color, float3(1.0, 0.0, 0.0), redFlash * (0.45 + death * 0.35));
        color = lerp(color, float3(0.0, 0.0, 0.0), blackFlash * (0.30 + death * 0.55));
        color = lerp(color, float3(1.0, 0.94, 0.82), whiteFlash * (0.25 + death * 0.75));
        color = lerp(color, float3(1.0, 0.96, 0.88), smoothstep(0.92, 1.0, death) * (0.48 + whiteFlash * 0.52));
    }
    return float4(saturate(color), 1.0);
}
)";
    }
