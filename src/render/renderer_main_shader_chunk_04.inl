R"(
float2 AspectSoftenedBloodUv(float2 uv, float3 worldPos)
{
    float2 duvDx = ddx(uv);
    float2 duvDy = ddy(uv);
    float det = duvDx.x * duvDy.y - duvDx.y * duvDy.x;
    if (abs(det) < 0.00001)
    {
        return uv;
    }
    float3 dpDx = ddx(worldPos);
    float3 dpDy = ddy(worldPos);
    float3 dpDu = (dpDx * duvDy.y - dpDy * duvDx.y) / det;
    float3 dpDv = (-dpDx * duvDy.x + dpDy * duvDx.x) / det;
    float uMeters = length(dpDu);
    float vMeters = length(dpDv);
    float minMeters = max(min(uMeters, vMeters), 0.05);
    float2 aspect = min(float2(uMeters, vMeters) / minMeters, float2(2.65, 2.65));
    aspect = lerp(float2(1.0, 1.0), aspect, 0.42);
    return (uv - 0.5) * aspect + 0.5;
}

float2 BloodUvWorldMeters(float2 uv, float3 worldPos)
{
    float2 duvDx = ddx(uv);
    float2 duvDy = ddy(uv);
    float3 dpDx = ddx(worldPos);
    float3 dpDy = ddy(worldPos);
    float det = duvDx.x * duvDy.y - duvDx.y * duvDy.x;
    if (abs(det) < 0.00000008)
    {
        float m0 = length(dpDx) / max(length(duvDx), 0.00001);
        float m1 = length(dpDy) / max(length(duvDy), 0.00001);
        float fallback = clamp(max(m0, m1), 0.08, 4.0);
        return float2(fallback, fallback);
    }
    float3 dpDu = (dpDx * duvDy.y - dpDy * duvDx.y) / det;
    float3 dpDv = (-dpDx * duvDy.x + dpDy * duvDx.x) / det;
    return clamp(float2(length(dpDu), length(dpDv)), float2(0.05, 0.05), float2(4.0, 4.0));
}

float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float Hash31(float3 p)
{
    p = frac(p * float3(127.1, 311.7, 74.7));
    p += dot(p, p.yzx + 19.19);
    return frac((p.x + p.y) * p.z);
}

float Len2Inf(float2 v)
{
    v = abs(v);
    return max(v.x, v.y);
}

float Noise3(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    float3 u = f * f * (3.0 - 2.0 * f);
    float n000 = Hash31(i + float3(0.0, 0.0, 0.0));
    float n100 = Hash31(i + float3(1.0, 0.0, 0.0));
    float n010 = Hash31(i + float3(0.0, 1.0, 0.0));
    float n110 = Hash31(i + float3(1.0, 1.0, 0.0));
    float n001 = Hash31(i + float3(0.0, 0.0, 1.0));
    float n101 = Hash31(i + float3(1.0, 0.0, 1.0));
    float n011 = Hash31(i + float3(0.0, 1.0, 1.0));
    float n111 = Hash31(i + float3(1.0, 1.0, 1.0));
    float nx00 = lerp(n000, n100, u.x);
    float nx10 = lerp(n010, n110, u.x);
    float nx01 = lerp(n001, n101, u.x);
    float nx11 = lerp(n011, n111, u.x);
    float nxy0 = lerp(nx00, nx10, u.y);
    float nxy1 = lerp(nx01, nx11, u.y);
    return lerp(nxy0, nxy1, u.z);
}

float Fbm3(float3 p)
{
    float v = 0.0;
    float a = 0.52;
    [loop]
    for (int i = 0; i < 5; ++i)
    {
        v += Noise3(p) * a;
        p = p * 2.07 + float3(17.1, 31.7, 11.3);
        a *= 0.50;
    }
    return v;
}
)"
