    static const char* OverlayShaderSource() {
        return R"(
struct OverlayVSIn
{
    float2 pos : POSITION;
    float4 color : COLOR0;
};

struct OverlayVSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

OverlayVSOut OverlayVS(OverlayVSIn input)
{
    OverlayVSOut o;
    o.pos = float4(input.pos, 0.0, 1.0);
    o.color = input.color;
    return o;
}

float4 OverlayPS(OverlayVSOut input) : SV_TARGET
{
    return input.color;
}
)";
    }

    static const char* TexturedOverlayShaderSource() {
        return R"(
Texture2D gOverlayTexture : register(t0);
SamplerState gOverlaySampler : register(s0);

struct TexturedOverlayVSIn
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct TexturedOverlayVSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

TexturedOverlayVSOut TexturedOverlayVS(TexturedOverlayVSIn input)
{
    TexturedOverlayVSOut o;
    o.pos = float4(input.pos, 0.0, 1.0);
    o.uv = input.uv;
    o.color = input.color;
    return o;
}

float4 TexturedOverlayPS(TexturedOverlayVSOut input) : SV_TARGET
{
    float4 texel = gOverlayTexture.Sample(gOverlaySampler, input.uv);
    return texel * input.color;
}
)";
    }
