R"(
Texture2D<float4> gLiquidColor : register(t23);
Texture2D<float> gLiquidDepth : register(t24);
cbuffer LiquidResolveConstants : register(b1)
{
    float4 gLiquidScale;
};

float4 PSLiquidResolve(float4 pos : SV_POSITION) : SV_TARGET
{
    // Replay the decal against the native scene depth. Match low-res samples to
    // this triangle's depth plane so a puddle cannot bleed onto a nearby wall.
    float2 sceneDepthSlope = float2(ddx(pos.z), ddy(pos.z));
    float2 liquidPos = pos.xy * gLiquidScale.xy - 0.5;
    int2 base = int2(floor(liquidPos));
    float2 f = frac(liquidPos);
    uint w, h;
    gLiquidDepth.GetDimensions(w, h);
    float4 result = 0.0;
    [unroll] for (int y = 0; y < 2; ++y)
    {
        [unroll] for (int x = 0; x < 2; ++x)
        {
            int2 texel = clamp(base + int2(x, y), int2(0, 0), int2(w, h) - 1);
            float2 offset = (float2(texel) - liquidPos) / gLiquidScale.xy;
            float expected = pos.z + dot(sceneDepthSlope, offset);
            float depth = gLiquidDepth.Load(int3(texel, 0));
            if (depth < 1.0 && abs(depth - expected) < 0.0000015)
            {
                float4 color = gLiquidColor.Load(int3(texel, 0));
                float weight = (x == 0 ? 1.0 - f.x : f.x) * (y == 0 ? 1.0 - f.y : f.y);
                result += float4(color.rgb * color.a, color.a) * weight;
            }
        }
    }
    clip(result.a - 0.001);
    return float4(result.rgb / result.a, result.a);
}
)"
