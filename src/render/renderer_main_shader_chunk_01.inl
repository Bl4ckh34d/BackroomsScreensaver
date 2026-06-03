R"(
    nointerpolation float material : TEXCOORD4;
};

VSOut VSMain(VSIn input)
{
    VSOut o;
    o.worldPos = input.pos;
    o.normal = input.normal;
    o.tangent = input.tangent;
    o.uv = input.uv;
    o.material = input.material;
    if (input.material > 11.05 && input.material < 11.45)
    {
        float seed = frac(input.material * 17.37);
        float time = gCameraPosTime.w;
        float vertical = saturate(input.uv.y);
        float edge = abs(input.uv.x - 0.5) * 2.0;
        float lowSpread = smoothstep(0.18, 1.0, vertical);
        float wave = sin(time * (0.62 + seed * 0.45) + input.pos.y * 2.7 + seed * 19.0);
        float flutter = sin(time * (1.41 + seed * 0.51) + input.pos.x * 1.8 + input.pos.z * 2.1);
        o.worldPos += input.normal * (wave * (0.055 + lowSpread * 0.115) + (1.0 - edge) * flutter * 0.038);
        o.worldPos += input.tangent * (flutter * (0.035 + lowSpread * 0.070));
        o.worldPos.y += sin(time * 0.52 + seed * 11.0 + input.uv.x * 4.2) * 0.028 * (1.0 - vertical * 0.35);
    }
    if (input.material > 20.40 && input.material < 20.95)
    {
        float seed = frac(input.material * 19.73);
        float time = gCameraPosTime.w;
        float along = input.uv.y;
        float ring = input.uv.x * 6.2831853;
        float2 meatUv = input.uv * float2(1.25, 2.65) + float2(seed * 0.37, seed * 0.19 + time * 0.006);
        float meatHeight = gNormalHeight.SampleLevel(gSampler, float3(meatUv, 15.0), 0.0).a;
        float meatRidge = (meatHeight - 0.48) * 2.0;
        float peristalsis = sin(along * 3.35 - time * 3.85 + seed * 11.0);
        float fine = sin(along * 8.7 + ring * 2.0 - time * 5.1 + seed * 23.0);
        float squeeze = peristalsis * 0.028 + fine * 0.009 + meatRidge * (0.010 + abs(peristalsis) * 0.011);
        float2 noiseP = float2(along * 3.15 - time * 0.10 + seed * 7.0, input.uv.x * 4.0 + time * 0.035 + seed * 5.0);
        float2 noiseCell = floor(noiseP);
        float2 noiseFrac = frac(noiseP);
        float2 noiseFade = noiseFrac * noiseFrac * noiseFrac * (noiseFrac * (noiseFrac * 6.0 - 15.0) + 10.0);
        float h00 = frac(sin(dot(noiseCell + float2(0.0, 0.0) + seed, float2(127.1, 311.7))) * 43758.5453);
        float h10 = frac(sin(dot(noiseCell + float2(1.0, 0.0) + seed, float2(127.1, 311.7))) * 43758.5453);
        float h01 = frac(sin(dot(noiseCell + float2(0.0, 1.0) + seed, float2(127.1, 311.7))) * 43758.5453);
        float h11 = frac(sin(dot(noiseCell + float2(1.0, 1.0) + seed, float2(127.1, 311.7))) * 43758.5453);
        float2 g00 = float2(cos(h00 * 6.2831853), sin(h00 * 6.2831853));
        float2 g10 = float2(cos(h10 * 6.2831853), sin(h10 * 6.2831853));
        float2 g01 = float2(cos(h01 * 6.2831853), sin(h01 * 6.2831853));
        float2 g11 = float2(cos(h11 * 6.2831853), sin(h11 * 6.2831853));
        float n00 = dot(g00, noiseFrac - float2(0.0, 0.0));
        float n10 = dot(g10, noiseFrac - float2(1.0, 0.0));
        float n01 = dot(g01, noiseFrac - float2(0.0, 1.0));
        float n11 = dot(g11, noiseFrac - float2(1.0, 1.0));
        float softNoise = lerp(lerp(n00, n10, noiseFade.x), lerp(n01, n11, noiseFade.x), noiseFade.y) * 1.35;
        float noisePulse = sin(time * 1.45 + along * 2.1 + seed * 31.0) * 0.5 + 0.5;
        float secondLayer = softNoise * (0.008 + noisePulse * 0.010);
        float crawl = sin(along * 1.85 - time * 1.35 + seed * 7.0) * 0.030;
        o.worldPos += input.normal * (squeeze + secondLayer);
        o.worldPos += input.tangent * (crawl + meatRidge * 0.006);
        o.worldPos.y += sin(along * 2.35 - time * 3.1 + seed * 13.0) * 0.025;
    }
    o.pos = mul(float4(o.worldPos, 1.0), gViewProj);
    return o;
}

VSOut VSInstanced(VSInstIn input)
{
    float3 axisX = input.instX.xyz;
    float3 axisY = input.instY.xyz;
    float3 axisZ = input.instZ.xyz;
    float3 origin = float3(input.instX.w, input.instY.w, input.instZ.w);

    VSIn baseInput;
    baseInput.pos = origin + axisX * input.pos.x + axisY * input.pos.y + axisZ * input.pos.z;
    float invLenX2 = rcp(max(0.000001, dot(axisX, axisX)));
    float invLenY2 = rcp(max(0.000001, dot(axisY, axisY)));
    float invLenZ2 = rcp(max(0.000001, dot(axisZ, axisZ)));
    baseInput.normal = normalize(axisX * input.normal.x * invLenX2 +
        axisY * input.normal.y * invLenY2 +
        axisZ * input.normal.z * invLenZ2);
    baseInput.tangent = normalize(axisX * input.tangent.x + axisY * input.tangent.y + axisZ * input.tangent.z);
    baseInput.uv = input.uv;
    baseInput.material = input.material;
    if (input.instMaterial.x >= 0.0)
    {
        baseInput.material = input.instMaterial.x;
    }
    else if (floor(baseInput.material) > 21.5 && floor(baseInput.material) < 22.5)
    {
        baseInput.material = 22.020 + fmod(abs(input.instMaterial.y), 0.92);
    }
    return VSMain(baseInput);
}

float MaterialId(float material)
{
)"
